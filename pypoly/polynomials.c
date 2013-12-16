#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "polynomials.h"

/**
 * Generic helpers
 */

#define MAX(a,b)    (((int)(a)>(int)(b))?(int)(a):(int)(b))

/* strdup is not part of the C standard and might not be available */
static inline char*
strduplicate(const char *s)
{
    int len = strlen(s) + 1;
    char *p;
    if ((p = malloc(len * sizeof(char))) == NULL) {
        return NULL;
    }
    return memcpy(p, s, len);
}
#define strdup strduplicate

/**
 * Complex numbers
 */

const Complex CZero = {0., 0.}, COne = {1., 0.};

#ifndef PYPOLY_VERSION
static inline Complex
complex_add(Complex a, Complex b)
{
    return (Complex){
        a.real + b.real,
        a.imag + b.imag
    };
}
static inline Complex
complex_sub(Complex a, Complex b)
{
    return (Complex){
        a.real - b.real,
        a.imag - b.imag
    };
}
static inline Complex
complex_neg(Complex z)
{
    return (Complex){
        - z.real,
        - z.imag
    };
}
static inline Complex
complex_mult(Complex a, Complex b)
{
    return (Complex){
        a.real * b.real - a.imag * b.imag,
        a.real * b.imag + a.imag * b.real
    };
}
static inline Complex
complex_div(Complex a, Complex b)
{
    Complex r;
    double d = b.real*b.real + b.imag*b.imag;
    if (d == 0.) {
        errno = EDOM;
    } else {
        r.real = (a.real*b.real + a.imag*b.imag)/d;
        r.imag = (a.imag*b.real - a.real*b.imag)/d;
    }
    return r;
}
#else
/* cPython comes with good implementations of operations on complex numbers.
 * Let's use them! */
#define complex_add _Py_c_sum
#define complex_sub _Py_c_diff
#define complex_neg _Py_c_neg
#define complex_mult _Py_c_prod
#define complex_div _Py_c_quot
#endif

/**
 * Polynomials
 */

/* This macro recomputes the degree of the Polynomial pointed by P.
 * To be called after some operation modifying the leading coefficient. */
#define Poly_ResizeDown(P)                                              \
    while ((P)->deg != -1 && complex_iszero((P)->coef[(P)->deg])) {     \
        --((P)->deg);                                                   \
    }

/* Create a Polynomial of degree "deg" at address pointed by P.
 * If "deg" is -1, no memory is allocated and the coefficients pointer
 * is set to NULL.
 *
 * /!\ This function assumes poly_set_coef will be called afterwards
 * so that the degree gets properly computed.
 */
int
poly_init(Polynomial *P, int deg)
{
    if (deg == -1) {
        P->coef = NULL;
    } else if ((P->coef = calloc(deg + 1, sizeof(Complex))) == NULL) {
        P->coef = NULL;
        return 0;
    }
    P->deg = deg;
    P->bloom = 0;
    return 1;
}

/* Free memory allocated for a polynomial */
void
poly_free(Polynomial *P)
{
    free(P->coef);
    P->coef = NULL;
}

static inline void
_poly_set_coef(Polynomial *P, int i, Complex c)
{
    if (!complex_iszero(c)) P->bloom |= Poly_BloomMask(i);
    P->coef[i] = c;
}

static inline void
_poly_incr_coef(Polynomial *P, int i, Complex c)
{
    if (!complex_iszero(c)) P->bloom |= Poly_BloomMask(i);
    P->coef[i].real += c.real;
    P->coef[i].imag += c.imag;
}

void
poly_set_coef(Polynomial *P, int i, Complex c)
{
    /* /!\ i should be <= allocated */
    _poly_set_coef(P, i, c);
    if (i > P->deg && !complex_iszero(c)) {
        P->deg = i;
    } else if (i == P->deg) {
        Poly_ResizeDown(P);
    }
}

/* Reallocate memory for P (e.g. for setting a new coef. higher than previous degree)
 *
 * /!\ This function assumes poly_set_coef will be called afterwards
 * so that the degree gets properly computed.
 */
int
poly_realloc(Polynomial *P, int deg)
{
    Complex *coef = realloc(P->coef, (deg + 1) * sizeof(Complex));
    if (coef == NULL) {
        return 0;
    }
    if (deg > P->deg) {
        memset(coef + P->deg + 1, 0, (deg - P->deg) * sizeof(Complex));
    }
    P->coef = coef;
    P->deg = deg;
    return 1;
}

int
poly_equal(Polynomial *P, Polynomial *Q)
{
    if (P == Q) return 1;
    if (P->deg != Q->deg) return 0;
    int i;
    for (i = 0; i <= P->deg; ++i) {
        if (P->coef[i].real != Q->coef[i].real
                ||
            P->coef[i].imag != Q->coef[i].imag) {
            return 0;
        }
    }
    return 1;
}

/* Polynomials string representation.
 * Examples:
 *      -1 + 3 * X**2
 *      -1+2.5j + (1+3j) * X
 * We traverse the coefficients and append characters to a buffer.
 * A little bit messy, but it seems to work.
 */
#define BUFFER_AVAILABLE(buffer, offset)           \
    ((int)sizeof(buffer)>(int)(offset))?           \
    ((int)sizeof(buffer)-(int)(offset))            \
    : 0

#define STR_UNKOWN              "X"
#define STR_J                   "j"
#define STR_TRUNCATED           "... [truncated]"

char*
poly_to_string(Polynomial *P)
{
    if (P->deg == -1) {
        return strdup("0");
    } else {
        char buffer[2048] = "";
        int i, multiplier, add_mult_sign, offset = 0;
        double re, im;
        for (i = 0; i <= P->deg; ++i) {
            if (complex_iszero(P->coef[i])) {
                continue;
            }

            multiplier = 1;
            add_mult_sign = 1;
            if (offset != 0) {
                multiplier = (P->coef[i].real <= 0 && P->coef[i].imag <= 0) ? -1 : 1;
                offset += snprintf(buffer + offset,
                                   BUFFER_AVAILABLE(buffer, offset),
                                   "%s", (multiplier == 1) ? " + " : " - ");
            }
            re = multiplier * P->coef[i].real;
            im = multiplier * P->coef[i].imag;
            if (P->coef[i].real == 0) {
                if (P->coef[i].imag != 1) {
                    offset += snprintf(buffer + offset,
                                       BUFFER_AVAILABLE(buffer, offset),
                                       "%g", im);
                }
                offset += snprintf(buffer + offset,
                                   BUFFER_AVAILABLE(buffer, offset),
                                   "%s", STR_J);
            } else if (im == 0) {
                if (re != 1 || i == 0) {
                    offset += snprintf(buffer + offset,
                                       BUFFER_AVAILABLE(buffer, offset),
                                       "%g", re);
                } else {
                    add_mult_sign = 0;
                }
            } else {
                offset += snprintf(buffer + offset,
                                   BUFFER_AVAILABLE(buffer, offset),
                                   i == 0 ? "%g%+g%s" : "(%g%+g%s)",
                                   re, im, STR_J);
            }
            if (i == 1) {
                offset += snprintf(buffer + offset,
                                   BUFFER_AVAILABLE(buffer, offset),
                                   "%s", add_mult_sign ? " * " STR_UNKOWN : STR_UNKOWN);
            } else if (i > 1) {
                offset += snprintf(buffer + offset,
                                   BUFFER_AVAILABLE(buffer, offset),
                                   "%s**%d", add_mult_sign ? " * " STR_UNKOWN : STR_UNKOWN, i);
            }
            if (offset > (int)sizeof(buffer)) {
                memcpy(buffer + sizeof(buffer) - strlen(STR_TRUNCATED) - 1,
                       STR_TRUNCATED, strlen(STR_TRUNCATED));
                break;
            }
        }
        return strdup(buffer);
    }
}

/* Polynomial evaluation at a given point using Horner's method.
 * Performs O(deg P) operations (naïve approach is quadratic).
 * See http://en.wikipedia.org/wiki/Horner%27s_method */
Complex
poly_eval(Polynomial *P, Complex c)
{
    Complex result = CZero;
    int i;
    for (i = P->deg; i >= 0; --i) {
        result = complex_add(complex_mult(result, c), P->coef[i]);
    }
    return result;
}

/**
 * Polynomial operators
 * We use the following naming convention:
 *  A, B, C, ... for the operators parameters
 *  P, Q, R, ... for the result destination
 *
 * The operators should assume the destination polynomials were NOT initiliazed.
 * It is up to the operators to perform this initialization when relevant.
 */

/* Copy polynomial pointed by A to the location pointed by P */
int
poly_copy(Polynomial *A, Polynomial *P)
{
    if (!poly_init(P, A->deg)) {
        return 0;
    }
    memcpy(P->coef, A->coef, (A->deg + 1) * sizeof(Complex));
    P->bloom = A->bloom;
    return 1;
}

int
poly_add(Polynomial *A, Polynomial *B, Polynomial *R)
{
    if (!poly_init(R, MAX(A->deg, B->deg))) {
        return 0;
    }
    int i;
    for (i = 0; i <= MAX(A->deg, B->deg); ++i) {
        _poly_set_coef(R, i, complex_add(Poly_GetCoef(A, i), Poly_GetCoef(B, i)));
    }
    Poly_ResizeDown(R);
    return 1;
}

int
poly_sub(Polynomial *A, Polynomial *B, Polynomial *R)
{
    if (!poly_init(R, MAX(A->deg, B->deg))) {
        return 0;
    }
    int i;
    for (i = 0; i <= MAX(A->deg, B->deg); ++i) {
        _poly_set_coef(R, i, complex_sub(Poly_GetCoef(A, i), Poly_GetCoef(B, i)));
    }
    Poly_ResizeDown(R);
    return 1;
}

int
poly_neg(Polynomial *A, Polynomial *Q)
{
    if (!poly_init(Q, A->deg)) {
        return 0;
    }
    int i;
    for (i = 0; i <= A->deg; ++i) {
        _poly_set_coef(Q, i, complex_neg(A->coef[i]));
    }
    return 1;
}

int
poly_scal_multiply(Polynomial *A, Complex c, Polynomial *R)
{
    if (complex_iszero(c)) {
        poly_init(R, -1);
        return 1;
    }
    if (!poly_init(R, A->deg)) {
        return 0;
    }
    int i;
    for (i = 0; i <= A->deg; ++i) {
        if (A->bloom & Poly_BloomMask(i)) {
            _poly_set_coef(R, i, complex_mult(Poly_GetCoef(A, i), c));
        }
    }
    return 1;
}

int
poly_multiply(Polynomial *A, Polynomial *B, Polynomial *R)
{
    // TODO: implement faster algo
    if (A->deg == -1 || B->deg == -1) {
        poly_init(R, -1);
        return 1;
    }
    if (!poly_init(R, A->deg + B->deg)) {
        return 0;
    }
    int i, j;
    for (i = 0; i <= A->deg + B->deg; ++i) {
        for (j = 0; j <= i; ++j) {
            if ((A->bloom & Poly_BloomMask(j)) && (B->bloom & Poly_BloomMask(i - j))) {
                _poly_incr_coef(R, i, complex_mult(Poly_GetCoef(A, j), Poly_GetCoef(B, i - j)));
            }
        }
    }
    return 1;
}

int
poly_pow(Polynomial *A, unsigned int n, Polynomial *R)
{
    if (n == 0) {
        int failure = 0;
        Poly_InitConst(R, ((Complex){1, 0}), failure);
        return !failure;
    }
    if (n == 1) {
        return poly_copy(A, R);
    }
    Polynomial T;
    if (!poly_multiply(A, A, &T)) return 0;
    if (!poly_pow(&T, n >> 1, R)) {
        poly_free(&T);
        return 0;
    }
    poly_free(&T);
    if (n & 1) {
        if (!poly_multiply(R, A, &T)) {
            poly_free(R);
            return 0;
        }
        poly_free(R);
        *R = T;
    }
    return 1;
}

int
poly_derive(Polynomial *A, unsigned int n, Polynomial *R)
{
    if (!poly_init(R, MAX(-1, A->deg - (int)n))) {
        return 0;
    }
    int i, j, multiplier;
    for (i = 0; i <= R->deg; ++i) {
        multiplier = 1;
        for (j = i; j < i + (int)n; ++j) multiplier *= j + 1;
        _poly_set_coef(R, i, complex_mult((Complex){(double)multiplier, 0}, A->coef[j]));
    }
    return 1;
}

int
poly_integrate(Polynomial *A, unsigned int n, Polynomial *R)
{
    if (!poly_init(R, (A->deg == -1) ? -1 : A->deg + (int)n)) {
        return 0;
    }
    int i, j, divisor;
    for (i = n; i <= R->deg; ++i) {
        divisor = 1;
        for (j = i; j > i - (int)n; --j) divisor *= j;
        _poly_set_coef(R, i, complex_div(A->coef[j], (Complex){(double)divisor, 0}));
    }
    return 1;
}

/* Euclidean division of A by B.
 * If B is not zero, the resulting polynomials Q and R are defined by:
 *      A = B * Q + R, deg R < deg B
 * If B is zero, the operation is undefined and returns -1.
 */
int
poly_div(Polynomial *A, Polynomial *B, Polynomial *Q, Polynomial *R)
{
    if (B->deg == -1) {
        return -1;  // Division by zero
    }
    Complex B_leadcoef = Poly_LeadCoef(B);
    Polynomial T1, T2;  // Used as buffers

    /* Polynomials coefficients MUST be initialized. */
    if (Q != NULL) {
        poly_init(Q, -1);
    }
    poly_init(R, -1);
    poly_init(&T1, -1);
    poly_init(&T2, -1);

    if (Q != NULL && !poly_init(Q, A->deg)) goto error;
    if (!poly_copy(A, R)) goto error;

    while (R->deg - B->deg >= 0) {
        if (!poly_init(&T1, R->deg - B->deg)) goto error;
        _poly_set_coef(&T1, R->deg - B->deg,
                      complex_div(Poly_LeadCoef(R), B_leadcoef));

        if (Q != NULL) {
            if (!poly_add(Q, &T1, &T2)) goto error;
            poly_free(Q);
            if (!poly_copy(&T2, Q)) goto error;
            poly_free(&T2);
        }

        if (!poly_multiply(&T1, B, &T2)) goto error;
        poly_free(&T1);
        if (!poly_sub(R, &T2, &T1)) goto error;
        poly_free(&T2);
        poly_free(R);
        if(!poly_copy(&T1, R)) goto error;
        poly_free(&T1);
    }
    return 1;
error:
    if (Q != NULL) poly_free(Q);
    poly_free(R);
    poly_free(&T1);
    poly_free(&T2);
    return 0;
}

/* Greatest Common Divisor of A and B.
 *
 * Computes the polynomial of highest degree which divides both A and B.
 * We add the condition that its leading coefficient should be 1 in order
 * to have a uniquely defined GCD.
 *
 * The algorithm is basically :
 *   # deg A >= deg B
 *   While B != 0
 *       A, B <= B, A % B   # Invariant: PGCD(A, B)
 *   P = A
 */
int
poly_gcd(Polynomial *A, Polynomial *B, Polynomial *P)
{
    Polynomial R, T;
    poly_init(P, -1);
    poly_init(&R, -1);
    poly_init(&T, -1);

    if (A->deg >= B->deg) {
        if (!poly_copy(A, P)) goto error;
        if (!poly_copy(B, &R)) goto error;
    } else {
        if (!poly_copy(B, P)) goto error;
        if (!poly_copy(A, &R)) goto error;
    }
    while (R.deg != -1) {
        if (!poly_copy(P, &T)) goto error;
        poly_free(P);
        if (!poly_copy(&R, P)) goto error;
        poly_free(&R);
        if (!poly_div(&T, P, NULL, &R)) goto error;
        poly_free(&T);
    }

    // Result normalization - could be faster.
    Complex factor = complex_div((Complex){1, 0}, Poly_LeadCoef(P));
    if (!poly_copy(P, &T)) goto error;
    poly_free(P);
    if (!poly_scal_multiply(&T, factor, P)) goto error;
    poly_free(&T);
    return 1;
error:
    poly_free(P);
    poly_free(&T);
    poly_free(&R);
    return 0;
}
