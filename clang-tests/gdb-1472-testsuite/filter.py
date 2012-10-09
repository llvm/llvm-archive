import re
import os
import sys

def readList(path):
    if not os.path.exists(path):
        return []
    f = open(path)
    lines = [ln.strip() for ln in f]
    f.close()
    return lines

if len(sys.argv) != 2:
    print "ignored failures directory must be provided"
    sys.exit(1)

if not os.path.exists(sys.argv[1]):
    print "ignored failures directory", sys.argv[1], "does not exist"

if not os.path.isdir(sys.argv[1]):
    print "ignored failures path", sys.argv[1], "is not a directory"


ignores = readList(os.path.join(sys.argv[1], 'FAIL.txt')) + \
          readList(os.path.join(sys.argv[1], 'UNRESOLVED.txt')) + \
          readList(os.path.join(sys.argv[1], 'XPASS.txt'))

testStateLineRE = re.compile(r'(FAIL|PASS|XFAIL|XPASS|UNRESOLVED): (.*)')

lines = sys.stdin.readlines()

for ln in lines:
    match = testStateLineRE.match(ln)
    if match is not None:
        code,name = match.groups()
        if name in ignores:
            code = 'IGNORE ' + code
        sys.stdout.write(code + ': ' + name + '\n')
    else:
        sys.stdout.write(ln)
