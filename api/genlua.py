#! /usr/bin/env python

import sys

from mpi.module import Module
from mpi.luawriter import Writer

assert len(sys.argv) == 2
mod = Module.from_xml(sys.argv[1])
Writer(sys.stdout).write(mod)
