pypoly: Python Polynomials
==========================

A Python extension written in C, implementing the `Polynomial` type.

*Requires Python 3*

*Work in progress*

Usage
-----

**Instanciating a polynomial:**

    >>> from pypoly import Polynomial
    >>> P = Polynomial(-1, 0, complex(1, 3))
    >>> P
    -1 + (1+3j) * X**2
    >>> P.degree
    2

Or:

    >>> from pypoly import X
    >>> P = -1 + (1+3j) * X**2
    >>> P
    -1 + (1+3j) * X**2

**Operations on polynomials:**

    >>> Polynomial(1, 2, 0.5) + Polynomial(2, 3)
    3 + 5 * X + 0.5 * X**2
    >>> Polynomial(-1, -1) * Polynomial(2, -3)
    -2 + X + 3 * X**2
    >>> P = Polynomial(1, 2, 3)
    >>> P(13)   # Evaluates P(x) for x = 13
    534.0

Even more:

    >>> (X - 1) * (X**2 + 2)
    -2 + 2 * X - X**2 + X**3
    >>> divmod(1 + X + X**2, 1 + X)     # Euclidean division
    (X, 1)
    >>> (2 * X + 3 * X**2 + X**5 + X**7) % (X**2 + 1)
    -3 + 2 * X
