class Writer(object):
    def __init__(self, out):
        super(Writer, self).__init__()
        self.out = out

    def write(self, module):
        self.module = module
        del self.module
