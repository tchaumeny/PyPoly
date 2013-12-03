from . import Polynomial, X

#
# Chebyshev polynomials
#

def ChebyshevIterator():
    U = 1
    yield U
    V = X
    yield V
    while True:
        T = V
        V = 2 * X * V - U
        yield V
        U = T

def Chebyshev(n):
    it = ChebyshevIterator()
    for i in range(n):
        next(it)
    return next(it)

#
# Legendre polynomials
#

def LegendreIterator():
    U = 1
    yield U
    V = X
    yield V
    n = 1
    while True:
        T = V
        V = 1 / (n + 1) * ((2 * n + 1) * X * V - n * U)
        yield V
        U = T
        n += 1 

def Legendre(n):
    it = LegendreIterator()
    for i in range(n):
        next(it)
    return next(it)
