#ifndef POLYNOMIALS_H
#define POLYNOMIALS_H

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
} Polynomial;

int poly_init(Polynomial *P, int deg);

void poly_free(Polynomial *P);

int poly_copy(Polynomial *P, Polynomial *R);

int poly_initX(Polynomial *P);

int poly_equal(Polynomial *P, Polynomial *Q);

char* poly_to_string(Polynomial *P);

void poly_set_coef(Polynomial *P, int i, Complex c);

Complex poly_eval(Polynomial *P, Complex c);

int poly_add(Polynomial *A, Polynomial *B, Polynomial *R);

int poly_sub(Polynomial *A, Polynomial *B, Polynomial *R);

int poly_neg(Polynomial *A, Polynomial *R);

int poly_scal_multiply(Polynomial *A, Complex c, Polynomial *R);

int poly_multiply(Polynomial *A, Polynomial *B, Polynomial *R);

int poly_pow(Polynomial *A, unsigned int n, Polynomial *R);

int poly_derive(Polynomial *A, Polynomial *R);

int poly_div(Polynomial *A, Polynomial *B, Polynomial *Q, Polynomial *R);

/* Macro to initialize the Polynomial pointed by P to the constant Polynomial c.
 * 'failure' flag is set to 1 in case of memory allocation error. */
#define Poly_InitConst(P, c, failure)           \
    if (poly_init((P), 0))                      \
        poly_set_coef((P), 0, (c));             \
    else                                        \
        failure = 1;

extern const Complex CZero, COne;

#define Poly_GetCoef(P, i)                                 \
    (((int)(i) > (P)->deg) ? CZero : (P)->coef[(int)(i)])

#define Poly_LeadCoef(P)        (((P)->deg==-1)?CZero:(P)->coef[(P)->deg])

#endif
