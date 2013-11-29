# Conf
PYTHON = python

.PHONY: clean install build pylint doctest unittest test benchmark check

clean:
	rm -rf build/ tests/__pycache__ MANIFEST

install:
	$(PYTHON) setup.py install

build: clean install

pylint:
	pylint tests/ benchmark.py setup.py --errors-only

doctest:
	$(PYTHON) -m doctest README.md -v

unittest:
	$(PYTHON) -m unittest

test: doctest unittest

benchmark:
	$(PYTHON) benchmark.py

check: pylint build test benchmark
