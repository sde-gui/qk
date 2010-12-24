import sys

__all__ = [ 'implement_me', 'oops' ]

def implement_me(what):
    print >> sys.stderr, 'implement me: %s' % what

def oops(message=None):
    raise RuntimeError(message)
