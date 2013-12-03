from distutils.core import setup, Extension
import sys

if sys.version_info[0] < 3:
    print("PyPoly requires Python >= 3")
    sys.exit()

pypoly_version = '0.2.1'

long_description = ""
with open('README.md') as f:
    for line in f:
      if "[Build Status]" not in line:
          long_description += line

_pypoly_module = Extension(
                    "_pypoly",
                    ["pypoly/polynomials.c", "pypoly/_pypoly.c"],
                    define_macros=[
                        ('PYPOLY_VERSION', pypoly_version)])

setup(name="PyPolynomial",
      description="Python polynomial C extension.",
      long_description=long_description,
      version=pypoly_version,
      packages=['pypoly'],
      ext_modules=[_pypoly_module,],
      author="Thomas Chaumeny",
      author_email="t.chaumeny@gmail.com",
      url="https://github.com/tchaumeny/PyPoly")
