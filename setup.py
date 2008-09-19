#!/usr/bin/env python

from distutils.core import setup, Extension
setup(name = "py_sg",
      version = "0.1",
      ext_modules=[
        Extension("py_sg", ["py_sg.c"])
        ],

      description = 'Python SCSI generic library',
      author = 'Dan Lenski',
      author_email = 'lenski@umd.edu',
      url = 'http://tonquil.homeip.net/~dlenski/py_sg',
      license = 'GPLv3'
)                                                                                                  
