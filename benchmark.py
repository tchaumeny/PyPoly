import sys
import time

from pypoly import Polynomial, X

sparse_polynomial1 = Polynomial(0, 0, -3, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5)
sparse_polynomial2 = Polynomial(0, 0, -3, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5, 0, 9)

dense_polynomial1 = Polynomial(1, -8, -3, 2, 5, 1, -1, 9, 8, 2, 3, 4, 5)
dense_polynomial2 = Polynomial(1, -8, -3, 2, 5, 1, -1, 9, 8, 2, 3, 4, 5, -7, 8)

def add_sparse():
    sparse_polynomial1 + sparse_polynomial2

def mult_sparse():
    sparse_polynomial1 * sparse_polynomial2

def power_sparse():
    sparse_polynomial1**3
    sparse_polynomial2**4

def compare_sparse():
    sparse_polynomial1 == sparse_polynomial2

def modulo_sparse():
    divmod(sparse_polynomial2, sparse_polynomial1)

def evaluate_sparse():
    sparse_polynomial1(10)

def add_dense():
    dense_polynomial1 + dense_polynomial2

def mult_dense():
    dense_polynomial1 * dense_polynomial2

def power_dense():
    dense_polynomial1**3
    dense_polynomial2**4

def compare_dense():
    dense_polynomial1 == dense_polynomial2

def modulo_dense():
    divmod(dense_polynomial2, dense_polynomial1)

def evaluate_dense():
    dense_polynomial1(10)

def X_op():
    1 + X + X**2 + X**3 + X**4 + X**5 + X**6

BENCHMARK = (
        (add_sparse, 10**7),
        (mult_sparse, 2 * 10**6),
        (power_sparse, 2 * 10**5),
        (modulo_sparse, 2 * 10**6),
        (compare_sparse, 2 * 10**7),
        (evaluate_sparse, 10**7),
        (add_dense, 10**7),
        (mult_dense, 2 * 10**6),
        (power_dense, 2 * 10**5),
        (modulo_dense, 10**6),
        (evaluate_dense, 10**7),
        (X_op, 10**6),
    )

def bench_func(func, times=None):
    if times is None:
        times = 10**6
    step = times // 100
    print("[ Calling \"%s\" %g times]" % (func.__name__, times))
    before = time.time()
    for i in range(times):
        func()
        if i % step == 0:
            sys.stdout.write(".")
            sys.stdout.flush()
    after = time.time()
    elapsed = after - before
    print("\n==> Done in %ss (average %ss)\n" % (elapsed, elapsed / times))
    return elapsed

def run():
    total = 0
    for func, times in BENCHMARK:
        total += bench_func(func, times)
    print("Total: %ss" % total)

if __name__ == '__main__':
    run()
