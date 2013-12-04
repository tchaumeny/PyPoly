import unittest

from pypoly import X
from pypoly.examples import *


class OnesTestCase(unittest.TestCase):
    def test_basic(self):
        self.assertEqual(Ones(3), 1 + X + X**2 + X**3)

class ChebyshevTestCase(unittest.TestCase):
    def test_chebyshev_0(self):
        self.assertEqual(Chebyshev(0), 1)

    def test_chebyshev_1(self):
        self.assertEqual(Chebyshev(1), X)

    def test_chebyshev_5(self):
        self.assertEqual(Chebyshev(5), 5 * X - 20 * X**3 + 16 * X**5)

class LegendreTestCase(unittest.TestCase):
    def test_legendre_0(self):
        self.assertEqual(Legendre(0), 1)

    def test_legendre_1(self):
        self.assertEqual(Legendre(1), X)

    def test_legendre_5(self):
        self.assertEqual(Legendre(5), (63 * X**5 - 70 * X**3 + 15 * X) / 8)

if __name__ == '__main__':
    unittest.main()
