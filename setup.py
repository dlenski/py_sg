#!/usr/bin/env python3

from distutils.core import setup, Extension
setup(
      packages=["py_sg"],
      ext_modules=[
        Extension("py_sg._py_sg", ["py_sg/_py_sg.c"])
      ],
      package_data={
        "py_sg": ["*.pyi", "py.typed"]
      },
)
