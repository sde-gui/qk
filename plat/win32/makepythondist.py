import os
import sys
import getopt
import shutil
import glob
import compileall
import zipfile

def remove_file(path, quiet=False):
    if os.path.isdir(path):
        if not quiet:
            print ' removing directory %s' % (path,)
        shutil.rmtree(path)
    elif os.path.exists(path):
        if not quiet:
            print ' removing file %s' % (path,)
        os.remove(path)

def remove_files(basedir, *args, **kwargs):
    for g in args:
        for path in glob.glob(os.path.join(basedir, g)):
            remove_file(path, quiet=kwargs.get('quiet', False))

def trim(pythondir):
    def make_path(*args):
        return os.path.join(pythondir, os.path.join(*args))
    def path_exists(relpath):
        return os.path.exists(make_path(relpath))
    if not os.path.isdir(pythondir):
        print >> sys.stderr, "Not a directory: '%s'" % (pythondir,)
        return 1
    if not os.path.isfile(make_path('python.exe')) or \
       not os.path.isdir(make_path('Lib')):
        print >> sys.stderr, "Directory '%s' does not look like a " + \
                             "Win32 Python directory" % (pythondir,)
        return 1

    for d in ('Doc', 'include', 'libs', 'Scripts', 'share', 'tcl'):
        remove_file(make_path(d))
    remove_file(make_path('NEWS.txt'))
    remove_files(pythondir, 'w9xpopen.exe', '*-wininst.log', 'Remove*.exe')

    remove_file(make_path('Tools', 'pynche'))

    for f in ('_ctypes_test.pyd', '_testcapi.pyd', '_tkinter.pyd',
              'tcl[0-9]*.dll', 'tclpip[0-9]*.dll', 'tk[0-9]*.dll'):
        remove_files(pythondir, os.path.join('DLLs', f))

    for f in ('bsddb\\test', 'ctypes\\test', 'curses', 'distutils',
              'email\\test', 'idlelib', 'json\\tests', 'lib2to3',
              'lib-tk', 'pkgconfig', 'sqlite3\\test', 'test',
              '__phello__.foo.py'):
        remove_file(make_path('Lib', f))

    for f in ('README.txt', '*.egg-info', '*\\*.egg-info', 'gtk-2.0\\codegen'):
        remove_files(pythondir, os.path.join('Lib', 'site-packages', f))

    for f in ('*.py[co]', '*\\*.py[co]', '*\\*\\*.py[co]', '*\\*\\*\\*.py[co]',
              '*\\*\\*\\*\\*.py[co]', '*\\*\\*\\*\\*\\*.py[co]'):
        remove_files(pythondir, f)

    return 0

def makedist(srcdir, destdir):
    if not os.path.isdir(srcdir):
        print >> sys.stderr, "Not a directory: '%s'" % (srcdir,)
        return 1
    if os.path.exists(destdir):
        print >> sys.stderr, "Directory '%s' already exists" % (destdir,)
        return 1

    def copyfiles(*args):
        if len(args) < 2:
            raise RuntimeError('oops')
        for src in args[:-1]:
            files = glob.glob(os.path.join(srcdir, src))
            if not files:
                raise RuntimeError("File '%s' does not exist in directory '%s'" % \
                                    (src, srcdir))
            for path in files:
                print ' copying %s to %s' % (path, args[-1])
                if os.path.isdir(path):
                    dest = os.path.join(destdir, args[-1], os.path.basename(path))
                    shutil.copytree(path, dest)
                else:
                    shutil.copy(path, os.path.join(destdir, args[-1]))

    dlls = glob.glob(os.path.join(srcdir, 'python[0-9][0-9].dll'))
    if not dlls:
        raise RuntimeError("No python dll in directory '%s'" % (srcdir,))
    if len(dlls) > 1:
        raise RuntimeError("Too many python dlls in directory '%s'" % (srcdir,))
    pydll, ext = os.path.splitext(os.path.basename(dlls[0]))
    assert ext == '.dll' and pydll.startswith('python')

    os.mkdir(destdir)
    os.mkdir(os.path.join(destdir, 'bin'))
    os.mkdir(os.path.join(destdir, 'lib'))
    os.mkdir(os.path.join(destdir, 'lib', 'python'))
    copyfiles('*.exe', '*.dll', 'bin')
    copyfiles('*.txt', 'lib\\python')
    copyfiles('DLLs', 'Tools', 'lib\\python')

    pylib_path = os.path.join(destdir, 'bin', 'Lib')
    pyzip_path = os.path.join(destdir, 'bin', pydll + '.zip')

    copyfiles('Lib', 'bin')
    print ' moving bin\\Lib\\site-packages to lib\\python\\'
    shutil.move(os.path.join(pylib_path, 'site-packages'),
                os.path.join(destdir, 'lib\\python'))

    shutil.copy(os.path.join(os.path.dirname(__file__), 'sitecustomize.py'),
                pylib_path)

    print " creating '%s'" % (pyzip_path,)
    libzip = zipfile.PyZipFile(pyzip_path, 'w', zipfile.ZIP_DEFLATED)
    libzip.writepy(pylib_path)
    for d in glob.glob(os.path.join(pylib_path, '*')):
        if os.path.isdir(d):
            libzip.writepy(d)
    libzip.close()

    print ' compiling python files in lib\\python\\site-packages'
    compileall.compile_dir(os.path.join(destdir, 'lib', 'python', 'site-packages'),
                           quiet=1)

    remove_file(pylib_path)


def main(argv):
    usage = '''\
usage: makepythondist.py makedist srcdir destdir
       makepythondist.py trim srcdir'''
    opts, args = getopt.getopt(argv[1:], "h", ["help"])
    for opt, arg in opts:
        if opt in ('-h', '--help'):
            print >> sys.stderr, usage
            return 1
        else:
            raise RuntimeError("oops")
    job = None
    if len(args) >= 1:
        job = args[0]
    if job == 'makedist':
        if len(args) == 3:
            return makedist(args[1], args[2])
    elif job == 'trim':
        if len(args) == 2:
            return trim(args[1])
    print >> sys.stderr, usage
    return 1

if __name__ == '__main__':
    sys.exit(main(sys.argv))
