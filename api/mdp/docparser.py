import re
import string
import sys
import os

DEBUG = False

class ParseError(RuntimeError):
    def __init__(self, message, block=None):
        if block:
            RuntimeError.__init__(self, '%s in file %s, around line %d' % \
                                    (message, block.filename, block.first_line))
        else:
            RuntimeError.__init__(self, message)

class DoxBlock(object):
    def __init__(self, block):
        object.__init__(self)

        self.symbol = None
        self.annotations = []
        self.params = []
        self.attributes = []
        self.docs = []

        self.__parse(block)

    def __parse(self, block):
        first_line = True
        chunks = []
        cur = [None, None, []]
        for line in block.lines:
            if first_line:
                m = re.match(r'([\w\d_.-]+(:+[\w\d_.-]+)*)(:(\s.*)?)?$', line)
                if m is None:
                    raise ParseError('bad id line', block)
                cur[0] = m.group(1)
                annotations, docs = self.__parse_annotations(m.group(4) or '')
                cur[1] = annotations
                cur[2] = [docs] if docs else []
            elif not line:
                if cur[0] is not None:
                    chunks.append(cur)
                    cur = [None, None, []]
                elif cur[2]:
                    cur[2].append(line)
            else:
                m = re.match(r'(@[\w\d_.-]+|Returns|Return value|Since):(.*)$', line)
                if m:
                    if cur[0] or cur[2]:
                        chunks.append(cur)
                        cur = [None, None, []]
                    cur[0] = m.group(1)
                    annotations, docs = self.__parse_annotations(m.group(2) or '')
                    cur[1] = annotations
                    cur[2] = [docs] if docs else []
                else:
                    cur[2].append(line)
            first_line = False
        if cur[0] or cur[2]:
            chunks.append(cur)

        self.symbol = chunks[0][0]
        self.annotations = chunks[0][1]

        for chunk in chunks[1:]:
            if chunk[0]:
                if chunk[0].startswith('@'):
                    self.params.append(chunk)
                else:
                    self.attributes.append(chunk)
            else:
                self.docs = chunk[2]

        if not self.symbol:
            raise ParseError('bad id line', block)

    def __parse_annotations(self, text):
        annotations = []
        ann_start = -1
        for i in xrange(len(text)):
            c = text[i]
            if c in ' \t':
                pass
            elif c == ':':
                if ann_start < 0:
                    if annotations:
                        return annotations, text[i+1:].strip()
                    else:
                        return None, text
                else:
                    pass
            elif c != '(' and ann_start < 0:
                if annotations:
                    raise ParseError('bad annotations')
                return None, text
            elif c == '(':
                if ann_start >= 0:
                    raise ParseError('( inside ()')
                ann_start = i
            elif c == ')':
                assert ann_start >= 0
                if ann_start + 1 < i:
                    annotations.append(text[ann_start+1:i])
                ann_start = -1
        if ann_start >= 0:
            raise ParseError('unterminated annotation')
        if annotations:
            return annotations, None
        else:
            return None, None

class Block(object):
    def __init__(self, lines, filename, first_line, last_line):
        object.__init__(self)
        self.lines = list(lines)
        self.filename = filename
        self.first_line = first_line
        self.last_line = last_line

class Symbol(object):
    def __init__(self, name, annotations, docs, block):
        object.__init__(self)
        self.name = name
        self.annotations = annotations
        self.docs = docs
        self.block = block

class Class(Symbol):
    def __init__(self, name, annotations, docs, block):
        Symbol.__init__(self, name, annotations, docs, block)

class Boxed(Symbol):
    def __init__(self, name, annotations, docs, block):
        Symbol.__init__(self, name, annotations, docs, block)

class Enum(Symbol):
    def __init__(self, name, annotations, docs, block):
        Symbol.__init__(self, name, annotations, docs, block)

class Flags(Symbol):
    def __init__(self, name, annotations, docs, block):
        Symbol.__init__(self, name, annotations, docs, block)

class Function(Symbol):
    def __init__(self, name, annotations, params, retval, docs, block):
        Symbol.__init__(self, name, annotations, docs, block)
        self.params = params
        self.retval = retval

class VMethod(Function):
    def __init__(self, name, annotations, params, retval, docs, block):
        Function.__init__(self, name, annotations, params, retval, docs, block)

class ParamBase(object):
    def __init__(self, annotations=None, docs=None):
        object.__init__(self)
        self.docs = docs
        self.type = None
        self.annotations = annotations

class Param(ParamBase):
    def __init__(self, name=None, annotations=None, docs=None):
        ParamBase.__init__(self, annotations, docs)
        self.name = name

class Retval(ParamBase):
    def __init__(self, annotations=None, docs=None):
        ParamBase.__init__(self, annotations, docs)

class Parser(object):
    def __init__(self):
        object.__init__(self)
        self.classes = []
        self.enums = []
        self.functions = []
        self.vmethods = []
        self.__classes_dict = {}
        self.__functions_dict = {}
        self.__vmethods_dict = {}

    def __split_block(self, block):
        chunks = []
        current_prefix = None
        current_annotations = None
        current_text = None
        first_line = True

        re_id = re.compile(r'([\w\d._-]+:(((\s*\([^()]\)\s*)+):)?')
        re_special = re.compile(r'(@[\w\d._-]+|(SECTION|PROPERTY|SIGNAL)-[\w\d._-]+||Since|Returns|Return value):(((\s*\([^()]\)\s*)+):)?')

        for line in block.lines:
            if first_line:
                line = re.sub(r'^SECTION:([\w\d_-]+):?', r'SECTION-\1:', line)
                line = re.sub(r'^([\w\d_-]+):([\w\d_-]+):?', r'PROPERTY-\1-\2:', line)
                line = re.sub(r'^([\w\d_-]+)::([\w\d_-]+):?', r'SIGNAL-\1-\2:', line)
            first_line = False
            if not line:
                if current_prefix is not None:
                    chunks.append([current_prefix, current_annotations, current_text])
                    current_prefix = None
                    current_annotations = None
                    current_text = None
                elif current_text is not None:
                    current_text.append(line)
            else:
                m = re_special.match(line)
                if m:
                    if current_prefix is not None or current_text is not None:
                        chunks.append([current_prefix, current_annotations, current_text])
                        current_prefix = None
                        current_annotations = None
                        current_text = None
                    current_prefix = m.group(1)
                    suffix = m.group(4) or ''
                    annotations, text = self.__parse_annotations(suffix)
                    current_annotations = annotations
                    current_text = [text] if text else []
                else:
                    if current_text is not None:
                        current_text.append(line)
                    else:
                        current_text = [line]
        if current_text is not None:
            chunks.append([current_prefix, current_annotations, current_text])

        return chunks

    def __parse_annotations(self, text):
        annotations = []
        ann_start = -1
        for i in xrange(len(text)):
            c = text[i]
            if c in ' \t':
                pass
            elif c == ':':
                if ann_start < 0:
                    if annotations:
                        return annotations, text[i+1:].strip()
                    else:
                        return None, text
                else:
                    pass
            elif c != '(' and ann_start < 0:
                if annotations:
                    raise ParseError('bad annotations')
                return None, text
            elif c == '(':
                if ann_start >= 0:
                    raise ParseError('( inside ()')
                ann_start = i
            elif c == ')':
                assert ann_start >= 0
                if ann_start + 1 < i:
                    annotations.append(text[ann_start+1:i])
                ann_start = -1
        if ann_start >= 0:
            raise ParseError('unterminated annotation')
        if annotations:
            return annotations, None
        else:
            return None, None

    def __parse_function(self, block):
        db = DoxBlock(block)

        params = []
        retval = None

        for p in db.params:
            params.append(Param(p[0][1:], p[1], p[2]))
        for attr in db.attributes:
            if attr[0] in ('Returns', 'Return value'):
                retval = Retval(attr[1], attr[2])
            elif attr[0] in ('Since'):
                pass
            else:
                raise ParseError('unknown attribute %s' % (attr[0],), block)

        if ':' in db.symbol:
            raise ParseError('bad function name %s' % (db.symbol,), block)

        func = Function(db.symbol, db.annotations, params, retval, db.docs, block)
        if DEBUG:
            print 'func.name:', func.name
        if func.name in self.__functions_dict:
            raise ParseError('duplicated function %s' % (func.name,), block)
        self.__functions_dict[func.name] = func
        self.functions.append(func)

    def __parse_class(self, block):
        db = DoxBlock(block)

        name = db.symbol
        if name.startswith('class:'):
            name = name[len('class:'):]

        if db.params:
            raise ParseError('class params', block)
        if db.attributes:
            raise ParseError('class attributes', block)

        cls = Class(name, db.annotations, db.docs, block)
        self.classes.append(cls)

    def __parse_boxed(self, block):
        db = DoxBlock(block)

        name = db.symbol
        if name.startswith('boxed:'):
            name = name[len('boxed:'):]

        if db.params:
            raise ParseError('boxed params', block)
        if db.attributes:
            raise ParseError('boxed attributes', block)

        cls = Boxed(name, db.annotations, db.docs, block)
        self.classes.append(cls)

    def __parse_enum(self, block):
        db = DoxBlock(block)

        name = db.symbol
        if name.startswith('enum:'):
            name = name[len('enum:'):]

        if db.params:
            raise ParseError('enum params', block)
        if db.attributes:
            raise ParseError('enum attributes', block)

        enum = Enum(name, db.annotations, db.docs, block)
        self.enums.append(enum)

    def __parse_flags(self, block):
        db = DoxBlock(block)

        name = db.symbol
        if name.startswith('flags:'):
            name = name[len('flags:'):]

        if db.params:
            raise ParseError('flags params', block)
        if db.attributes:
            raise ParseError('flags attributes', block)

        flags = Flags(name, db.annotations, db.docs, block)
        self.enums.append(flags)

    def __parse_block(self, block):
        line = block.lines[0]
        if line.startswith('class:'):
            self.__parse_class(block)
        elif line.startswith('boxed:'):
            self.__parse_boxed(block)
        elif line.startswith('enum:'):
            self.__parse_enum(block)
        elif line.startswith('flags:'):
            self.__parse_flags(block)
        elif line.startswith('SECTION:'):
            pass
        else:
            self.__parse_function(block)

    def __add_block(self, block, filename, first_line, last_line):
        lines = []
        for line in block:
            if line.startswith('*'):
                line = line[1:].strip()
            if line or lines:
                lines.append(line)
            else:
                first_line += 1
        i = len(lines) - 1
        while i >= 0:
            if not lines[i]:
                del lines[i]
                i -= 1
                last_line -= 1
            else:
                break
        if lines:
            assert last_line >= first_line
            self.__parse_block(Block(lines, filename, first_line, last_line))

    def __read_comments(self, filename):
        block = None
        first_line = 0
        line_no = 0
        for line in open(filename):
            line = line.strip()
            if not block:
                if line.startswith('/**'):
                    line = line[3:]
                    if not line.startswith('*') and not '*/' in line:
                        block = [line]
                        first_line = line_no
            else:
                end = line.find('*/')
                if end >= 0:
                    line = line[:end]
                    block.append(line)
                    self.__add_block(block, filename, first_line, line_no)
                    block = None
                else:
                    block.append(line)
            line_no += 1
        if block:
            raise ParseError('unterminated block in file %s' % (filename,))

    def read_files(self, filenames):
        for f in filenames:
            print >> sys.stderr, 'parsing gtk-doc comments in file', f
            self.__read_comments(f)
        for f in filenames:
            print >> sys.stderr, 'parsing declarations in file', f
            self.__read_declarations(f)

    # Code copied from h2def.py by Toby D. Reeves <toby@max.rl.plh.af.mil>

    def __strip_comments(self, buf):
        parts = []
        lastpos = 0
        while 1:
            pos = string.find(buf, '/*', lastpos)
            if pos >= 0:
                if buf[pos:pos+len('/**vtable:')] != '/**vtable:':
                    parts.append(buf[lastpos:pos])
                    pos = string.find(buf, '*/', pos)
                    if pos >= 0:
                        lastpos = pos + 2
                    else:
                        break
                else:
                    parts.append(buf[lastpos:pos+len('/**vtable:')])
                    lastpos = pos + len('/**vtable:')
            else:
                parts.append(buf[lastpos:])
                break
        return string.join(parts, '')

    # Strips the dll API from buffer, for example WEBKIT_API
    def __strip_dll_api(self, buf):
        pat = re.compile("[A-Z]*_API ")
        buf = pat.sub("", buf)
        return buf

    def __clean_func(self, buf):
        """
        Ideally would make buf have a single prototype on each line.
        Actually just cuts out a good deal of junk, but leaves lines
        where a regex can figure prototypes out.
        """
        # bulk comments
        buf = self.__strip_comments(buf)

        # dll api
        buf = self.__strip_dll_api(buf)

        # compact continued lines
        pat = re.compile(r"""\\\n""", re.MULTILINE)
        buf = pat.sub('', buf)

        # Preprocess directives
        pat = re.compile(r"""^[#].*?$""", re.MULTILINE)
        buf = pat.sub('', buf)

        #typedefs, stucts, and enums
        pat = re.compile(r"""^(typedef|struct|enum)(\s|.|\n)*?;\s*""",
                         re.MULTILINE)
        buf = pat.sub('', buf)

        #strip DECLS macros
        pat = re.compile(r"""G_(BEGIN|END)_DECLS|(BEGIN|END)_LIBGTOP_DECLS""", re.MULTILINE)
        buf = pat.sub('', buf)

        #extern "C"
        pat = re.compile(r"""^\s*(extern)\s+\"C\"\s+{""", re.MULTILINE)
        buf = pat.sub('', buf)

        #multiple whitespace
        pat = re.compile(r"""\s+""", re.MULTILINE)
        buf = pat.sub(' ', buf)

        #clean up line ends
        pat = re.compile(r""";\s*""", re.MULTILINE)
        buf = pat.sub('\n', buf)
        buf = buf.lstrip()

        #associate *, &, and [] with type instead of variable
        #pat = re.compile(r'\s+([*|&]+)\s*(\w+)')
        pat = re.compile(r' \s* ([*|&]+) \s* (\w+)', re.VERBOSE)
        buf = pat.sub(r'\1 \2', buf)
        pat = re.compile(r'\s+ (\w+) \[ \s* \]', re.VERBOSE)
        buf = pat.sub(r'[] \1', buf)

        buf = string.replace(buf, '/** vtable:', '/**vtable:')
        pat = re.compile(r'(\w+) \s* \* \s* \(', re.VERBOSE)
        buf = pat.sub(r'\1* (', buf)

        # make return types that are const work.
        buf = re.sub(r'\s*\*\s*G_CONST_RETURN\s*\*\s*', '** ', buf)
        buf = string.replace(buf, 'G_CONST_RETURN ', 'const-')
        buf = string.replace(buf, 'const ', 'const-')

        #strip GSEAL macros from the middle of function declarations:
        pat = re.compile(r"""GSEAL""", re.VERBOSE)
        buf = pat.sub('', buf)

        return buf

    def __read_declarations_in_buf(self, buf, filename):
        vproto_pat=re.compile(r"""
        /\*\*\s*vtable:(?P<vtable>[\w\d_]+)\s*\*\*/\s*
        (?P<ret>(-|\w|\&|\*)+\s*)  # return type
        \s+                   # skip whitespace
        \(\s*\*\s*(?P<vfunc>\w+)\s*\)
        \s*[(]   # match the function name until the opening (
        \s*(?P<args>.*?)\s*[)]     # group the function arguments
        """, re.IGNORECASE|re.VERBOSE)
        proto_pat=re.compile(r"""
        (?P<ret>(-|\w|\&|\*)+\s*)  # return type
        \s+                   # skip whitespace
        (?P<func>\w+)\s*[(]   # match the function name until the opening (
        \s*(?P<args>.*?)\s*[)]     # group the function arguments
        """, re.IGNORECASE|re.VERBOSE)
        arg_split_pat = re.compile("\s*,\s*")

        buf = self.__clean_func(buf)
        buf = string.split(buf,'\n')

        for p in buf:
            if not p:
                continue

            if DEBUG:
                print 'matching line', repr(p)

            fname = None
            vfname = None
            m = proto_pat.match(p)
            if m is None:
                if DEBUG:
                    print 'proto_pat not matched'
                m = vproto_pat.match(p)
                if m is None:
                    if DEBUG:
                        print 'vproto_pat not matched'
                        if p.find('vtable:') >= 0:
                            print "oops", repr(p)
                        if p.find('moo_file_enc_new') >= 0:
                            print '***', repr(p)
                    continue
                else:
                    vfname = m.group('vfunc')
                    if DEBUG:
                        print 'proto_pat matched', repr(m.group(0))
                        print '+++ vfname', vfname
            else:
                if DEBUG:
                    print 'proto_pat matched', repr(m.group(0))
                fname = m.group('func')
            ret = m.group('ret')
            if ret in ('return', 'else', 'if', 'switch'):
                continue
            if fname:
                func = self.__functions_dict.get(fname)
                if func is None:
                    continue
                if DEBUG:
                    print 'match:|%s|' % fname
            else:
                func = self.__vmethods_dict.get(vfname)
                if func is None:
                    func = VMethod('virtual:%s:%s' % (m.group('vtable'), m.group('vfunc')), None, None, None, None, None)
                    self.__vmethods_dict[func.name] = func
                    self.vmethods.append(func)
                if DEBUG:
                    print 'match:|%s|' % func.name

            args = m.group('args')
            args = arg_split_pat.split(args)
            for i in range(len(args)):
                spaces = string.count(args[i], ' ')
                if spaces > 1:
                    args[i] = string.replace(args[i], ' ', '-', spaces - 1)

            if ret != 'void':
                if func.retval is None:
                    func.retval = Retval()
                if func.retval.type is None:
                    func.retval.type = ret

            is_varargs = 0
            has_args = len(args) > 0
            for arg in args:
                if arg == '...':
                    is_varargs = 1
                elif arg in ('void', 'void '):
                    has_args = 0
            if DEBUG:
                print 'func ', ', '.join([p.name for p in func.params] if func.params else '')
            if has_args and not is_varargs:
                if func.params is None:
                    func.params = []
                elif func.params:
                    assert len(func.params) == len(args)
                for i in range(len(args)):
                    if DEBUG:
                        print 'arg:', args[i]
                    argtype, argname = string.split(args[i])
                    if DEBUG:
                        print argtype, argname
                    if len(func.params) <= i:
                        func.params.append(Param())
                    if func.params[i].name is None:
                        func.params[i].name = argname
                    if func.params[i].type is None:
                        func.params[i].type = argtype
            if DEBUG:
                print 'func ', ', '.join([p.name for p in func.params])

    def __read_declarations(self, filename):
        if DEBUG:
            print filename
        buf = open(filename).read()
        self.__read_declarations_in_buf(buf, filename)
