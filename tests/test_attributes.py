import unittest
import sys

from pypoly import Polynomial, X

class DegreeTestCase(unittest.TestCase):
    def test_zero(self):
        self.assertEqual(Polynomial(0).degree, -1)

    def test_const(self):
        self.assertEqual(Polynomial(1).degree, 0)

    def test_X(self):
        self.assertEqual(X.degree, 1)

    def test_polynomial(self):
        self.assertEqual((1 + X - 3 * X**4).degree, 4)

    def test_readonly(self):
        Exc = AttributeError if sys.version_info[0] >= 3 else TypeError
        with self.assertRaises(Exc):
            X.degree = 3

if __name__ == '__main__':
    unittest.main()
