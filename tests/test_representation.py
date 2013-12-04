import unittest

import pypoly

class RepresentationTestCase(unittest.TestCase):

    def test_constant_zero(self):
        self.assertEqual(repr(pypoly.Polynomial(0)), "0")

    def test_constant_int(self):
        self.assertEqual(repr(pypoly.Polynomial(1)), "1")

    def test_constant_float(self):
        self.assertEqual(repr(pypoly.Polynomial(1.5)), "1.5")

    def test_constant_complex(self):
        self.assertEqual(repr(pypoly.Polynomial(complex(1, 2))), "1+2j")

    def test_X(self):
        self.assertEqual(repr(pypoly.X), "X")

    def test_polynomial1(self):
        self.assertEqual(
            repr(pypoly.Polynomial(complex(-1, 2.5), complex(1, 3))),
            "-1+2.5j + (1+3j) * X")

    def test_polynomial2(self):
        self.assertEqual(
            repr(pypoly.Polynomial(-1, 0, 3)),
            "-1 + 3 * X**2")

    def test_polynomial3(self):
        self.assertEqual(
            repr(pypoly.Polynomial(complex(0, -0.2), complex(-2, -3))),
            "-0.2j - (2+3j) * X")

    def test_polynomial4(self):
        self.assertEqual(
            repr(pypoly.Polynomial(1, -3, 0, complex(0, -0.2))),
            "1 - 3 * X - 0.2j * X**3")

    def test_overflow(self):
        """We use a finite buffer (typically, 2048 chars) to construct the repr
        of a polynomial. The representation should be truncated to prevent buffer
        overflow."""
        long_repr = repr(pypoly.Polynomial(*(1 for _ in range(1000))))
        self.assertEqual(len(long_repr), 2047)
        self.assertEqual(
            long_repr,
            ("1 + X + " + " + ".join("X**%d" % i for i in range(2, 1000)))[:2032]
            + "... [truncated]"
        )

if __name__ == '__main__':
    unittest.main()
