from distutils.core import setup, Extension
import re
import sys


with open("pypoly/version.py") as f:
    vfile_content = f.read()

match = re.search(r"__version__ = '([^']+)'", vfile_content)
assert match

__version__ = match.group(1)

long_description = ""
with open("README.md") as f:
    for line in f:
      if "[Build Status]" not in line:
          long_description += line

_pypoly_module = Extension(
                    "_pypoly",
                    ["pypoly/polynomials.c", "pypoly/_pypoly.c"],
                    define_macros=[
                        ('PYPOLY_VERSION', __version__)])

setup(name="PyPolynomial",
      description="Python polynomial C extension.",
      long_description=long_description,
      version=__version__,
      packages=["pypoly"],
      ext_modules=[_pypoly_module,],
      author="Thomas Chaumeny",
      author_email="t.chaumeny@gmail.com",
      url="https://github.com/tchaumeny/PyPoly",
      classifiers=[
          'Development Status :: 4 - Beta',
          'Intended Audience :: Developers',
          'Intended Audience :: Education',
          'Intended Audience :: Science/Research',
          'License :: OSI Approved :: MIT License',
          'Programming Language :: C',
          'Programming Language :: Python',
          'Topic :: Scientific/Engineering :: Mathematics',
          ],)
