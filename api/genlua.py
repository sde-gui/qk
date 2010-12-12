#! /usr/bin/env python

import sys
import optparse

from mpi.module import Module
from mpi.luawriter import Writer

op = optparse.OptionParser()
op.add_option("-i", "--import", action="append", dest="import_modules")
op.add_option("--include-header", action="append", dest="include_headers")
(opts, args) = op.parse_args()

import_modules = []
if opts.import_modules:
    for filename in opts.import_modules:
        import_modules.append(Module.from_xml(filename))

assert len(args) == 1
mod = Module.from_xml(args[0])
for im in import_modules:
    mod.import_module(im)
Writer(sys.stdout).write(mod, opts.include_headers)
