import subprocess
import sys
import os

#print "Output file:", sys.argv[1]
#print "Working dir:", os.getcwd()
#print "Executing command:", sys.argv[2:]
output = subprocess.check_output(sys.argv[2:], stdin=None, shell=False, universal_newlines=False)
output = output.replace('\r\n', '\n').replace('\r', '\n')

filename = sys.argv[1]
tmp = filename + '.tmp'
with open(tmp, 'w') as f:
    f.write(output)
if os.path.exists(filename):
    os.remove(filename)
os.rename(tmp, filename)
