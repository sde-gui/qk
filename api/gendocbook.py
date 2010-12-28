#! /usr/bin/env python

import sys
import optparse

from mpi.module import Module
from mpi.docbookwriter import Writer

op = optparse.OptionParser()
op.add_option("--python", action="store_true")
op.add_option("--lua", action="store_true")
op.add_option("--template", action="store")
(opts, args) = op.parse_args()

assert len(args) == 1
assert bool(opts.python) + bool(opts.lua) == 1
if opts.python:
    mode = 'python'
elif opts.lua:
    mode = 'lua'

mod = Module.from_xml(args[0])
Writer(mode, opts.template, sys.stdout).write(mod)
