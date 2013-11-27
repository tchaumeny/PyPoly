import unittest

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
        with self.assertRaises(AttributeError):
            X.degree = 3

if __name__ == '__main__':
    unittest.main()
