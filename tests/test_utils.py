import unittest

import pypoly

class PolyXTestCase(unittest.TestCase):
    def test_X_type(self):
        self.assertTrue(isinstance(pypoly.X, pypoly.Polynomial))

    def test_X_value(self):
        self.assertEqual(pypoly.X, pypoly.Polynomial(0, 1))

if __name__ == '__main__':
    unittest.main()
