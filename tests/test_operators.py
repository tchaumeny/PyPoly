import unittest

from pypoly import Polynomial, X

class ComparisonTestCase(unittest.TestCase):
    def test_same_obj(self):
        self.assertTrue(X == X)
        self.assertFalse(X != X)

    def test_zero(self):
        self.assertTrue(Polynomial() == 0)
        self.assertFalse(Polynomial() != 0)

    def test_same_poly(self):
        self.assertTrue(Polynomial(1, 1) == Polynomial(1, 1))
        self.assertFalse(Polynomial(1, 1) != Polynomial(1, 1))

    def test_diff_poly1(self):
        self.assertFalse(Polynomial(1, 2) == Polynomial(1, 1))
        self.assertTrue(Polynomial(1, 2) != Polynomial(1, 1))

    def test_diff_poly2(self):
        self.assertFalse(Polynomial(1, 1, 1) == Polynomial(1, 1))
        self.assertTrue(Polynomial(1, 1, 1) != Polynomial(1, 1))

    def test_error_lt(self):
        with self.assertRaises(TypeError):
            X < X

    def test_error_gt(self):
        with self.assertRaises(TypeError):
            X > X

    def test_error_le(self):
        with self.assertRaises(TypeError):
            X <= X

    def test_error_ge(self):
        with self.assertRaises(TypeError):
            X >= X

class AdditionTestCase(unittest.TestCase):
    def test_constant_float(self):
        self.assertEqual(Polynomial(1, 2) + 0.5, Polynomial(1.5, 2))

    def test_constant_complex(self):
        self.assertEqual(Polynomial(1, 2) + complex(0, 0.5),
            Polynomial(complex(1, 0.5), 2))

    def test_polynomials(self):
        self.assertEqual(Polynomial(1, 2, 0.5) + Polynomial(2, 3),
            3 + 5 * X + 0.5 * X**2)

    def test_error_incompatible(self):
        with self.assertRaises(TypeError):
            X + {}

class MultiplicationTestCase(unittest.TestCase):
    def test_polynomials(self):
        self.assertEqual((1 + X + 2 * X**2) * (complex(-2, 1) * X - 2),
            -2 + complex(-4, 1) * X + (-6+1j) * X**2 + complex(-4, 2) * X**3)

    def test_error_incompatible(self):
        with self.assertRaises(TypeError):
            X * {}

class PowerTestCase(unittest.TestCase):
    def test_polynomials(self):
        self.assertEqual((1 + X)**2, 1 + 2 * X + X**2)


class DivmodTestCase(unittest.TestCase):
    def test_polynomials_divmod(self):
        self.assertEqual(divmod(1 + X + X**2, X + 1), (X, 1))

    def test_polynomials_mod(self):
        self.assertEqual((1 + X + X**2) % (X + 1), 1)

    def test_divzero_error(self):
        with self.assertRaises(ZeroDivisionError):
            X % 0


if __name__ == '__main__':
    unittest.main()
