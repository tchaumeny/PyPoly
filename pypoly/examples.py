from pypoly import Polynomial, X

#
# 1 + X + X**2 + ...
#

def Ones(n):
    return Polynomial(*(1 for _ in range(n + 1)))

#
# Chebyshev polynomials
#

def ChebyshevIterator():
    U = 1
    yield U
    V = X
    yield V
    while True:
        U, V = V, 2 * X * V - U
        yield V

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
        U, V = V, 1. / (n + 1) * ((2 * n + 1) * X * V - n * U)
        yield V
        n += 1

def Legendre(n):
    it = LegendreIterator()
    for i in range(n):
        next(it)
    return next(it)

#
# Hermite polynomials
#

def HermiteIterator():
    U = 1
    yield U
    V = X
    yield V
    n = 1
    while True:
        U, V = V, X * V - n * U
        yield V
        n += 1

def Hermite(n):
    it = HermiteIterator()
    for i in range(n):
        next(it)
    return next(it)

#
# Cyclotomic polynomials
#  
    
def Cyclotomic(n):
    if n < 1:
        raise Exception("only positive") 
    elif n == 1:
        return X - 1

    denominator = X - 1
    for d in range(2,n):
        if n%d == 0:
           denominator *= Cyclotomic(d)  
    
    return (X**n - 1) // denominator
