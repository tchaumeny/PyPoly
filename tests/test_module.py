import unittest

from pypoly import *

class PolyXTestCase(unittest.TestCase):
    def test_X_type(self):
        self.assertTrue(isinstance(X, Polynomial))

    def test_X_value(self):
        self.assertEqual(X, Polynomial(0, 1))

class GCDTestCase(unittest.TestCase):
    def test_zero(self):
        self.assertEqual(
            gcd(Polynomial(1, 2, 3), Polynomial(0)),
            Polynomial(1. / 3, 2. / 3, 1))

    def test_constant(self):
        self.assertEqual(
            gcd(Polynomial(1, 2, 3), Polynomial(5)),
            Polynomial(1))

    def test_cyclotomic(self):
        self.assertEqual(
            gcd(X**6 - 1, X**9 - 1),
            X**3 - 1)

    def test_factors(self):
        self.assertEqual(
            gcd((1 + X)**2 * (2 + X) * (4 + X), (1 + X) * (2 + X) * (3 + X)),
            (1 + X) * (2 + X))

if __name__ == '__main__':
    unittest.main()
