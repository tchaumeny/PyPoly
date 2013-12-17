==========================
pypoly: Python Polynomials 
==========================

A Python extension written in C, implementing the ``Polynomial`` type.

Installation
============

Installation from source::

    $ python setup.py install

Or, using pip_::

    $ pip install PyPolynomial

.. _pip: http://www.pip-installer.org

Usage
=====

**Instanciating a polynomial:**

.. code-block:: python

    >>> from pypoly import Polynomial
    >>> P = Polynomial(-1, 0, complex(1, 3))
    >>> P
    -1 + (1+3j) * X**2
    >>> P.degree
    2

Or:

.. code-block:: python

    >>> from pypoly import X
    >>> P = (X - 1) * (X**2 + 2)
    >>> P
    -2 + 2 * X - X**2 + X**3

**Operations on polynomials:**

.. code-block:: python

    >>> Polynomial(-1, -1) * Polynomial(2, -3)
    -2 + X + 3 * X**2
    >>> P = Polynomial(1, 2, 3)
    >>> P(13)
    534.0
    >>> (1 + X + X**2) // (1 + X)
    X
    >>> (2 * X + 3 * X**2 + X**5 + X**7) % (X**2 + 1)
    -3 + 2 * X

Even more:

.. code-block:: python

    >>> (1 + X**3 - 2 * X**5) >> 2      # Second derivative
    6 * X - 40 * X**3
    >>> (1 + 2 * X) << 1                # Primitive integral
    X + X**2
    >>> from pypoly import gcd
    >>> gcd(X**6 - 1, X**12 - 1, X**9 - 1)
    -1 + X**3

Links
=====

`PyPoly tracker`_ (bugs, feature requests, etc.)

`Project page on PyPI`_

`Project page on GitHub`_ |buildstatus|

.. _`PyPoly tracker`: https://github.com/tchaumeny/PyPoly/issues/

.. _`Project page on PyPI`: https://pypi.python.org/pypi/PyPolynomial

.. _`Project page on GitHub`: https://github.com/tchaumeny/PyPoly

.. |buildstatus| image:: https://travis-ci.org/tchaumeny/PyPoly.png
   :alt: Build Status
   :target: https://travis-ci.org/tchaumeny/PyPoly
