import unittest
import sys

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

    def test_nonreg_zeromultiply(self):
        """Non-regression, multiplication by zero did not set degree to -1."""
        self.assertEqual(0 * X, 0)

class AdditionTestCase(unittest.TestCase):
    def test_positive_op(self):
        self.assertEqual(+X, X)

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

class SubstractionTestCase(unittest.TestCase):
    def test_negative_op(self):
        self.assertEqual(-Polynomial(-1, -1), Polynomial(1, 1))

    def test_constant_float(self):
        self.assertEqual(Polynomial(1, 2) - 0.5, Polynomial(0.5, 2))

    def test_constant_complex(self):
        self.assertEqual(Polynomial(1, 2) - complex(0, 0.5),
            Polynomial(complex(1, -0.5), 2))

    def test_polynomials(self):
        self.assertEqual(Polynomial(1, 2, 0.5) - Polynomial(2, 3),
            -1 - X + 0.5 * X**2)

    def test_error_incompatible(self):
        with self.assertRaises(TypeError):
            X - {}

class MultiplicationTestCase(unittest.TestCase):
    def test_constant(self):
        self.assertEqual(2 * Polynomial(1, 1), Polynomial(2, 2))

    def test_polynomials(self):
        self.assertEqual((1 + X + 2 * X**2) * (complex(-2, 1) * X - 2),
            -2 + complex(-4, 1) * X + (-6+1j) * X**2 + complex(-4, 2) * X**3)

    def test_error_incompatible(self):
        with self.assertRaises(TypeError):
            X * {}

class DivisionTestCase(unittest.TestCase):
    def test_polynomials(self):
        self.assertEqual(X / 1j, - 1j * X)

    def test_error_divide_by_poly(self):
        with self.assertRaises(TypeError):
            1 / X

    def test_zerodiverror(self):
        with self.assertRaises(ZeroDivisionError):
            X / 0

class PowerTestCase(unittest.TestCase):
    def test_positive(self):
        self.assertEqual((1 + X)**2, 1 + 2 * X + X**2)

    def test_zero(self):
        self.assertEqual((1 + X)**0, 1)

    def test_error_high_exponent(self):
        funcname = 'assertRaisesRegex' if sys.version_info[0] >= 3 else 'assertRaisesRegexp'
        assertRaisesRegex = getattr(self, funcname)
        with assertRaisesRegex(ValueError,
            """Polynomial exponentiation with exponents higher"""
            """ than 1024 is not supported"""):
            X**1025

    def test_error_neg(self):
        with self.assertRaises(TypeError):
            (1 + X)**-1

    def test_error_pow_poly(self):
        with self.assertRaises(TypeError):
            X**X

class DivmodTestCase(unittest.TestCase):
    def test_polynomials_divmod(self):
        self.assertEqual(divmod(1 + X + X**2, X + 1), (X, 1))

    def test_polynomials_mod(self):
        self.assertEqual((1 + X + X**2) % (X + 1), 1)

    def test_polynomials_eucdiv(self):
        self.assertEqual((1 + X + X**2) // (X + 1), X)

    def test_divzero_error(self):
        with self.assertRaises(ZeroDivisionError):
            X % 0

class SequenceTestCase(unittest.TestCase):
    def test_get_item(self):
        self.assertEqual((1 + 2 * X + 3 * X**2)[1], 2)

    def test_get_item_gt_deg(self):
        self.assertEqual((1 + 2 * X + 3 * X**2)[3], 0)

    def test_assign_item(self):
        P = 1 + 2 * X + 3 * X**2
        P[2] = 4.5
        self.assertEqual(P, 1 + 2 * X + 4.5 * X**2)
        self.assertEqual(P.degree, 2)

    def test_assign_item_larger(self):
        P = 1 + 2 * X + 3 * X**2
        P[8] = 1j
        self.assertEqual(P, 1 + 2 * X + 3 * X**2 + 1j * X**8)
        self.assertEqual(P.degree, 8)

    def test_assign_item_zero(self):
        P = 1 + 2 * X + 3 * X**2
        P[2] = 0
        self.assertEqual(P, 1 + 2 * X)
        self.assertEqual(P.degree, 1)

    def test_assign_item_zero_larger(self):
        P = 1 + 2 * X + 3 * X**2
        P[8] = 0
        self.assertEqual(P, 1 + 2 * X + 3 * X**2)
        self.assertEqual(P.degree, 2)

class CallTestCase(unittest.TestCase):
    def test_constant(self):
        self.assertEqual(Polynomial(1, 0, 0)(2), 1)

    def test_polynomials(self):
        self.assertEqual(Polynomial(1, 2, 3)(2), 17)

    def test_error_incompatible(self):
        with self.assertRaises(TypeError):
            Polynomial({})

class DerivationTestCase(unittest.TestCase):
    def test_derive_zero(self):
        self.assertEqual(Polynomial(0) >> 1, 0)

    def test_constant(self):
        self.assertEqual(Polynomial(1) >> 1, 0)

    def test_polynomials(self):
        self.assertEqual(Polynomial(1, 2, 3) >> 1, Polynomial(2, 6))

    def test_zero(self):
        self.assertEqual(Polynomial(1, 2, 3) >> 0, Polynomial(1, 2, 3))

    def test_error_negative(self):
        with self.assertRaises(TypeError):
            X >> -1

class IntegrationTestCase(unittest.TestCase):
    def test_integrate_zero(self):
        self.assertEqual(Polynomial(0) << 1, 0)

    def test_constant(self):
        self.assertEqual(Polynomial(1) << 1, X)

    def test_polynomials(self):
        self.assertEqual(Polynomial(1, 2, 3) << 1, Polynomial(0, 1, 1, 1))

    def test_zero(self):
        self.assertEqual(Polynomial(1, 2, 3) << 0, Polynomial(1, 2, 3))

    def test_error_negative(self):
        with self.assertRaises(TypeError):
            X << -1

if __name__ == '__main__':
    unittest.main()
