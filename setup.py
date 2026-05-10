#!/usr/bin/env python3

from distutils.core import setup, Extension
setup(
      ext_modules=[
        Extension("py_sg", ["py_sg.c"])
      ],
)
