#include <Python.h>
#include <structmember.h>

#include "polynomials.h"

/* A Python Polynomial Object */
typedef struct {
    PyObject_HEAD
    Polynomial poly;
} PyPoly_PolynomialObject;

static PyTypeObject PyPoly_PolynomialType;  // Forward declaration

/* Classic macro to check if a PyObject is a Polynomial */
#define PyPolynomial_Check(op) PyObject_TypeCheck((op), &PyPoly_PolynomialType)

/* Create a new Python Polynomial object.
 * If a pointer to a Polynomial is given as parameter, the pointed Polynomial
 * will be copied into the PyObject and the "deg" parameter will be ignored.
 * /!\ This will transfer ownership of the coefficients pointer /!\
 *
 * If not, a new Polynomial of degree "deg" will be allocated, initialized to 0.
 */
static PyPoly_PolynomialObject*
new_poly_st(PyTypeObject *subtype, int deg, Polynomial *P)
{
    PyPoly_PolynomialObject *self;
    self = (PyPoly_PolynomialObject *) (subtype->tp_alloc(subtype, 0));
    if (self != NULL) {
        if (P == NULL) {
            if(!poly_init(&(self->poly), deg)) {
                Py_DECREF(self);
                return (PyPoly_PolynomialObject*)PyErr_NoMemory();
            }
        } else {
            self->poly = *P;
        }
    }
    return self;
}
#define NewPoly(deg, P)     new_poly_st(&PyPoly_PolynomialType, (int)(deg), (Polynomial*)(P))

/* Polynomial extraction helpers.
 * Those functions deal with the problem of getting a Polynomial object out
 * of an arbitrary PyObject.
 *
 * The macro ExtractOrBorrowPoly(obj, P, status) will try to borrow a Polynomial
 * from "obj", if "obj" is a Python Polynomial.
 * Otherwise, it will try and create a new Polynomial, if possible
 * (a Python number will give a constant Polynomial).
 * The "extracted" Polynomial will be stored in destination pointed by "P"
 * and the status flag will be set accordingly.
 */
typedef enum {
   EXTRACT_CREATED,      // Polynomial created
   EXTRACT_BORROWED,     // Polynomial "borrowed" from object
   EXTRACT_ERR,          // Generic error handled by cPython (overflow, etc.)
   EXTRACT_ERRTYPE,      // Unsupported type
   EXTRACT_ERRMEM        // Malloc failure
} PolyExtractionStatus;

#define PolyExtractionFailure(status) ((PolyExtractionStatus)(status) >= EXTRACT_ERR)

PolyExtractionStatus
extract_poly(PyObject *obj, Polynomial *P)
{
    int failure = 0;
    if (PyLong_Check(obj)) {
        Py_complex c = CZero;
        c.real = PyLong_AsDouble(obj);
        if (c.real == -1.0 && PyErr_Occurred()) {
            return EXTRACT_ERR;
        }
        Poly_InitConst(P, c, failure)
        return failure ? EXTRACT_ERRMEM : EXTRACT_CREATED;
    }
    if (PyFloat_Check(obj)) {
        Py_complex c = CZero;
        c.real = PyFloat_AsDouble(obj);
        if (c.real == -1.0 && PyErr_Occurred()) {
            return EXTRACT_ERR;
        }
        Poly_InitConst(P, c, failure)
        return failure ? EXTRACT_ERRMEM : EXTRACT_CREATED;
    }
    if (PyComplex_Check(obj)) {
        Py_complex c = PyComplex_AsCComplex(obj);
        if (c.real == -1.0 && PyErr_Occurred()) {
            return EXTRACT_ERR;
        }
        Poly_InitConst(P, c, failure)
        return failure ? EXTRACT_ERRMEM : EXTRACT_CREATED;
    }
    return EXTRACT_ERRTYPE;
}

#define ExtractOrBorrowPoly(obj, P, status)             \
    if (PyPolynomial_Check(obj)) {                      \
        P = ((PyPoly_PolynomialObject*)obj)->poly;      \
        status = EXTRACT_BORROWED;                      \
    } else {                                            \
        status = extract_poly(obj, &P);                 \
    }

/**
 * PyObject API implementation
 */

static PyObject*
PyPoly_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
    PyPoly_PolynomialObject *self;
    int size = PyTuple_GET_SIZE(args);
    self = new_poly_st(subtype, size - 1, NULL);
    if (self != NULL) {
        Py_complex c;
        int i;
        for (i = 0; i < size; ++i) {
            c = PyComplex_AsCComplex(PyTuple_GET_ITEM(args, i));
            if (c.real == -1.0 && PyErr_Occurred()) {
                Py_DECREF(self);
                return NULL;
            }
            poly_set_coef(&(self->poly), i, c);
        }
    }
    return (PyObject*)self;
}

static void
PyPoly_dealloc(PyPoly_PolynomialObject *self)
{
    poly_free(&(self->poly));
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject*
PyPoly_copy(PyPoly_PolynomialObject *self)
{
    Polynomial P;
    if (!poly_copy(&(self->poly), &P)) {
        return PyErr_NoMemory();
    }
    return (PyObject*)NewPoly(0, &P);
}

static PyObject*
PyPoly_repr(PyPoly_PolynomialObject *self)
{
    char* str = poly_to_string(&(self->poly));
    return PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, str, strlen(str));
}

/* Macro used to generate binary functions on Python objects
 * using functions defined on Polynomial objects.
 * Takes care of Polynomial extraction from parameters and possible errors.
 * REFACTORING NEEDED /!\ */
#define PYPOLY_BINARYFUNC(poly_func)                        \
    int A_status, B_status;                                 \
    Polynomial A, B;                                        \
    ExtractOrBorrowPoly(self, A, A_status)                  \
    ExtractOrBorrowPoly(other, B, B_status)                 \
    if (PolyExtractionFailure(A_status)                     \
        ||                                                  \
        PolyExtractionFailure(B_status)) {                  \
        if (A_status == EXTRACT_CREATED) poly_free(&A);     \
        if (B_status == EXTRACT_CREATED) poly_free(&B);     \
        if (A_status == EXTRACT_ERRTYPE                     \
            ||                                              \
            B_status == EXTRACT_ERRTYPE) {                  \
            Py_RETURN_NOTIMPLEMENTED;                       \
        } else {                                            \
            return PyErr_NoMemory();                        \
        }                                                   \
    }                                                       \
    Polynomial R;                                           \
    if (!poly_func(&A, &B, &R)) {                           \
        if (A_status == EXTRACT_CREATED) poly_free(&A);     \
        if (B_status == EXTRACT_CREATED) poly_free(&B);     \
        return PyErr_NoMemory();                            \
    }                                                       \
    return (PyObject*)NewPoly(0, &R);

static PyObject*
PyPoly_add(PyObject *self, PyObject *other)
{
    PYPOLY_BINARYFUNC(poly_add)
}

static PyObject*
PyPoly_sub(PyObject *self, PyObject *other)
{
    PYPOLY_BINARYFUNC(poly_sub)
}

static PyObject*
PyPoly_mult(PyObject *self, PyObject *other)
{
    PYPOLY_BINARYFUNC(poly_multiply)
}

static PyObject*
PyPoly_neg(PyPoly_PolynomialObject *self)
{
    Polynomial P;
    if (!poly_neg(&(self->poly), &P)) {
        return PyErr_NoMemory();
    }
    return (PyObject*)NewPoly(0, &P);
}

static PyObject*
PyPoly_call(PyPoly_PolynomialObject *self, PyObject *args, PyObject *kw)
{
    Py_complex x;

    if (!_PyArg_NoKeywords("__call__()", kw) || !PyArg_ParseTuple(args, "D", &x)) {
        return NULL;
    }
    Py_complex y = poly_eval(&(self->poly), x);
    if (y.imag == 0) {
        return PyFloat_FromDouble(y.real);
    }
    return PyComplex_FromCComplex(y);
}

static PyObject*
PyPoly_pow(PyPoly_PolynomialObject *self, PyObject *pyexp, PyObject *pymod)
{
    unsigned long exponent = PyLong_AsUnsignedLong(pyexp);
    if (exponent == -1 && PyErr_Occurred()) {
        Py_RETURN_NOTIMPLEMENTED;
    }
    Polynomial P;
    if (!poly_pow(&(self->poly), exponent, &P)) {
        return PyErr_NoMemory();
    }
    return (PyObject*)NewPoly(0, &P);
}

static PyObject*
PyPoly_remain(PyObject *self, PyObject *other)
{
    int A_status, B_status;
    Polynomial A, B;
    ExtractOrBorrowPoly(self, A, A_status)
    ExtractOrBorrowPoly(other, B, B_status)
    if (PolyExtractionFailure(A_status)
        ||
        PolyExtractionFailure(B_status)) {
        if (A_status == EXTRACT_CREATED) poly_free(&A);
        if (B_status == EXTRACT_CREATED) poly_free(&B);
        if (A_status == EXTRACT_ERRTYPE
            ||
            B_status == EXTRACT_ERRTYPE) {
            Py_RETURN_NOTIMPLEMENTED;
        } else {
            return PyErr_NoMemory();
        }
    }
    Polynomial R;
    int res = poly_div(&A, &B, NULL, &R);
    if (res != 1) {
        if (A_status == EXTRACT_CREATED) poly_free(&A);
        if (B_status == EXTRACT_CREATED) poly_free(&B);
        if (res == -1) {
            PyErr_SetString(PyExc_ZeroDivisionError,
                            "Polynomial Euclidean division by"
                            " zero is undefined");
            return NULL;
        }
        return PyErr_NoMemory();
    }
    PyObject *p;
    if ((p = (PyObject*)NewPoly(0, &R)) == NULL) {
        return NULL;
    }
    return p;
}

static PyObject*
PyPoly_divmod(PyObject *self, PyObject *other)
{
    int A_status, B_status;
    Polynomial A, B;
    ExtractOrBorrowPoly(self, A, A_status)
    ExtractOrBorrowPoly(other, B, B_status)
    if (PolyExtractionFailure(A_status)
        ||
        PolyExtractionFailure(B_status)) {
        if (A_status == EXTRACT_CREATED) poly_free(&A);
        if (B_status == EXTRACT_CREATED) poly_free(&B);
        if (A_status == EXTRACT_ERRTYPE
            ||
            B_status == EXTRACT_ERRTYPE) {
            Py_RETURN_NOTIMPLEMENTED;
        } else {
            return PyErr_NoMemory();
        }
    }
    Polynomial Q, R;
    int res = poly_div(&A, &B, &Q, &R);
    if (res != 1) {
        if (A_status == EXTRACT_CREATED) poly_free(&A);
        if (B_status == EXTRACT_CREATED) poly_free(&B);
        if (res == -1) {
            PyErr_SetString(PyExc_ZeroDivisionError,
                            "Polynomial Euclidean division by"
                            " zero is undefined");
            return NULL;
        }
        return PyErr_NoMemory();
    }
    PyObject *p1, *p2, *t;
    if ((p1 = (PyObject*)NewPoly(0, &Q)) == NULL) {
        return NULL;
    }
    if ((p2 = (PyObject*)NewPoly(0, &R)) == NULL) {
        Py_DECREF(p1);
        return NULL;
    }
    if ((t = PyTuple_Pack(2, p1, p2)) == NULL) {
        Py_DECREF(p1);
        Py_DECREF(p2);
        return NULL;
    }
    return t;
}

static PyObject*
PyPoly_compare(PyObject *self, PyObject *other, int opid)
{
    if (opid != Py_EQ && opid != Py_NE) {
        Py_RETURN_NOTIMPLEMENTED;
    }
    int A_status, B_status;
    Polynomial A, B;
    ExtractOrBorrowPoly(self, A, A_status)
    ExtractOrBorrowPoly(other, B, B_status)
    if (PolyExtractionFailure(A_status)
        ||
        PolyExtractionFailure(B_status)) {
        if (A_status == EXTRACT_CREATED) poly_free(&A);
        if (B_status == EXTRACT_CREATED) poly_free(&B);
        if (A_status == EXTRACT_ERRTYPE
            ||
            B_status == EXTRACT_ERRTYPE) {
            Py_RETURN_NOTIMPLEMENTED;
        } else {
            return PyErr_NoMemory();
        }
    }
    if ((poly_equal(&A, &B) && opid == Py_EQ)
            ||
        (!poly_equal(&A, &B) && opid == Py_NE)) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject*
PyPoly_getitem(PyPoly_PolynomialObject *self, Py_ssize_t i)
{
    Py_complex coef = Poly_GetCoef(&(self->poly), i);
    if (coef.imag == 0) {
        return PyFloat_FromDouble(coef.real);
    }
    return PyComplex_FromCComplex(coef);
}

static PyMemberDef
PyPoly_members[] = {
   {"degree", T_INT, offsetof(PyPoly_PolynomialObject, poly) + offsetof(Polynomial, deg),
    READONLY, "The degree of the Polynomial instance." },
   { NULL }
};

static PyNumberMethods PyPoly_NumberMethods = {
     (binaryfunc)PyPoly_add,    /* nb_add */
     (binaryfunc)PyPoly_sub,    /* nb_subtract */
     (binaryfunc)PyPoly_mult,   /* nb_multiply */
     (binaryfunc)PyPoly_remain, /* nb_remainder */
     (binaryfunc)PyPoly_divmod, /* nb_divmod */
     (ternaryfunc)PyPoly_pow,   /* nb_power */
     (unaryfunc)PyPoly_neg,     /* nb_negative */
     (unaryfunc)PyPoly_copy     /* nb_positive */
     /*
     unaryfunc nb_absolute;
     inquiry nb_bool;
     unaryfunc nb_invert;
     binaryfunc nb_lshift;
     binaryfunc nb_rshift;
     binaryfunc nb_and;
     binaryfunc nb_xor;
     binaryfunc nb_or;
     unaryfunc nb_int;
     void *nb_reserved;
     unaryfunc nb_float;

     binaryfunc nb_inplace_add;
     binaryfunc nb_inplace_subtract;
     binaryfunc nb_inplace_multiply;
     binaryfunc nb_inplace_remainder;
     ternaryfunc nb_inplace_power;
     binaryfunc nb_inplace_lshift;
     binaryfunc nb_inplace_rshift;
     binaryfunc nb_inplace_and;
     binaryfunc nb_inplace_xor;
     binaryfunc nb_inplace_or;

     binaryfunc nb_floor_divide;
     binaryfunc nb_true_divide;
     binaryfunc nb_inplace_floor_divide;
     binaryfunc nb_inplace_true_divide;

     unaryfunc nb_index;*/
};

static PySequenceMethods PyPoly_as_sequence = {
    0,  /* sq_length */
    0,  /* sq_concat */
    0,  /* sq_repeat */
    (ssizeargfunc)PyPoly_getitem,
    0,
    0,  /* sq_ass_item */
    0,
};

static PyTypeObject PyPoly_PolynomialType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Polynomial",             /* tp_name */
    sizeof(PyPoly_PolynomialObject), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyPoly_dealloc,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)PyPoly_repr,                         /* tp_repr */
    &PyPoly_NumberMethods,                         /* tp_as_number */
    &PyPoly_as_sequence,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    (ternaryfunc)PyPoly_call,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Polynomial objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    (richcmpfunc)PyPoly_compare, /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    NULL,           /* tp_methods */
    PyPoly_members,           /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    (newfunc)PyPoly_new,                         /* tp_new */
};

static PyModuleDef PyPolymodule = {
    PyModuleDef_HEAD_INIT,
    "pypoly",
    "Python Polynomial module implemented in C.",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_pypoly(void) 
{
    PyObject* m;

    if (PyType_Ready(&PyPoly_PolynomialType) < 0)
        return NULL;

    m = PyModule_Create(&PyPolymodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&PyPoly_PolynomialType);
    PyModule_AddObject(m, "Polynomial", (PyObject *)&PyPoly_PolynomialType);

    /* Make "X" available for import */
    Polynomial X;
    if (!poly_initX(&X)) {
        return PyErr_NoMemory();
    }
    PyPoly_PolynomialObject *PyPoly_X;
    if ((PyPoly_X = NewPoly(0, &X)) == NULL) {
        return NULL;
    }
    Py_INCREF(PyPoly_X);
    PyModule_AddObject(m, "X", (PyObject *)PyPoly_X);
    return m;
}