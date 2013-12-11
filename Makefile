# Conf
PYTHON = python

.PHONY: clean install build pylint doctest unittest test benchmark

clean:
	rm -rf build/ tests/__pycache__ MANIFEST

install:
	CFLAGS="-g -Wall -Wextra -pedantic -std=c99" $(PYTHON) setup.py install

build: clean install

pylint:
	pylint pypoly tests benchmark.py --errors-only

doctest:
	$(PYTHON) -m doctest README.rst -v

unittest:
	$(PYTHON) -m unittest discover -s tests

test: doctest unittest

benchmark:
	$(PYTHON) benchmark.py

release:
	$(PYTHON) setup.py sdist upload
