import os
import sys

__all__ = []

try:
    # it is supposed to be a zip file
    filename = __file__
    mydir = os.path.dirname(filename)
    if mydir.lower().endswith('.zip'):
        mydir = os.path.dirname(mydir)
        if os.path.basename(mydir).lower() == 'bin':
            libdir = os.path.join(os.path.dirname(mydir), 'lib', 'python')
            sys.path.insert(1, os.path.join(libdir, 'site-packages', 'gtk-2.0'))
            sys.path.insert(1, os.path.join(libdir, 'site-packages'))
            sys.path.insert(1, os.path.join(libdir, 'DLLs'))
except:
    # oh well...
    pass
