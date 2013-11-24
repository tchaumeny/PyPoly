from distutils.core import setup, Extension

pypoly_version = '0.1.0'

pypoly_module = Extension(
                    "pypoly",
                    ["pypoly/polynomials.c", "pypoly/pypoly.c"],
                    define_macros=[
                        ('PYPOLY_VERSION', pypoly_version)])

setup(name="PyPolynomial", version=pypoly_version, ext_modules=[pypoly_module,])
