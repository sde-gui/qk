import sys

__all__ = [ 'implement_me', 'oops', 'warning' ]

def implement_me(what):
    print >> sys.stderr, 'implement me: %s' % what

def oops(message=None):
    raise RuntimeError(message)

def warning(message):
    print >>sys.stderr, "WARNING:", message
