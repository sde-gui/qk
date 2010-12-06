#
#  cproj/parser.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

import re

errors = [
    r'In file included from (?P<file>[^:]+):(?P<line>\d+):.*',
    r'(?P<file>[^:]+):(?P<line>\d+):(\d+:)?\s*error\s*:.*',
    r'(?P<file>[^:]+):(?P<line>\d+):.*',
]
warnings = [
    r'(?P<file>[^:]+):(?P<line>\d+):(\d+:)?\s*warning\s*:.*',
]

errors = [re.compile(e) for e in errors]
warnings = [re.compile(e) for e in warnings]


class ErrorInfo(object):
    def __init__(self, file, line, type='error'):
        object.__init__(self)
        self.file = file
        self.line = line - 1
        self.type = type

def parse_make_error(line):
    for p in errors:
        m = p.match(line)
        if not m:
            continue
        return ErrorInfo(m.group('file'), int(m.group('line')))
    for p in warnings:
        m = p.match(line)
        if not m:
            continue
        return ErrorInfo(m.group('file'), int(m.group('line')), 'warning')
    return None
