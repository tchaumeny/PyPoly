#ifndef POLYNOMIALS_H
#define POLYNOMIALS_H

#include <stdint.h>

#ifndef PYPOLY_VERSION
typedef struct {
    double real;
    double imag;
} Complex;
#else
#include <Python.h>
#define Complex Py_complex
#endif

/* Polynomial structure.
 * A Polynomial is represented as a basic array.
 * Since a Complex generally takes 8 bytes of memory, the coefficients will take
 * (1 + degree) * 8 bytes of memory. This shouldn't be a problem in common
 * use cases. */
typedef struct {
    Complex* coef;
    int deg;
    uint32_t bloom;
} Polynomial;

int poly_init(Polynomial *P, int deg);

void poly_free(Polynomial *P);

int poly_copy(Polynomial *P, Polynomial *R);

int poly_equal(Polynomial *P, Polynomial *Q);

char* poly_to_string(Polynomial *P);

void poly_set_coef(Polynomial *P, int i, Complex c);

int poly_realloc(Polynomial *P, int deg);

Complex poly_eval(Polynomial *P, Complex c);

int poly_add(Polynomial *A, Polynomial *B, Polynomial *R);

int poly_sub(Polynomial *A, Polynomial *B, Polynomial *R);

int poly_neg(Polynomial *A, Polynomial *R);

int poly_scal_multiply(Polynomial *A, Complex c, Polynomial *R);

int poly_multiply(Polynomial *A, Polynomial *B, Polynomial *R);

int poly_pow(Polynomial *A, unsigned int n, Polynomial *R);

int poly_derive(Polynomial *A, unsigned int n, Polynomial *R);

int poly_integrate(Polynomial *A, unsigned int n, Polynomial *R);

int poly_div(Polynomial *A, Polynomial *B, Polynomial *Q, Polynomial *R);

int poly_gcd(Polynomial *A, Polynomial *B, Polynomial *P);

/* Common Macros / inline helpers */

/* Check if a complex number equals (0,0).
 * We ignore double precision related errors. */
static inline int complex_iszero(Complex c) {
    return (c.real == 0. && c.imag == 0.);
}
/* Macro to initialize the Polynomial pointed by P to the constant Polynomial c.
 * 'failure' flag is set to 1 in case of memory allocation error. */
#define Poly_InitConst(P, c, failure)           \
    if (complex_iszero(c))                      \
        poly_init((P), -1);                     \
    else if (poly_init((P), 0))                 \
        poly_set_coef((P), 0, (c));             \
    else                                        \
        failure = 1;

extern const Complex CZero, COne;

#define Poly_BloomMask(i)                       \
    (0x00000001 << ((i) & 0x0000001f))

#define Poly_GetCoef(P, i)                                          \
    (((int)(i) > (P)->deg || !(Poly_BloomMask(i) & (P)->bloom))     \
        ? CZero : (P)->coef[(int)(i)])

#define Poly_LeadCoef(P)                        \
    (((P)->deg==-1)?CZero:(P)->coef[(P)->deg])

#endif
