#!/usr/bin/python

import sys

if len(sys.argv) != 2:
    sys.stderr.write('error: invalid argument count\n')
    sys.exit(1)

var_name = sys.argv[1]

sys.stdout.write('static char %s[] =' % (var_name))

tstr = sys.stdin.readline()

while tstr != '':
    tstr = tstr.rstrip()
    sys.stdout.write('\n "%s\\n"' % (tstr))
    tstr = sys.stdin.readline()

sys.stdout.write(';\n\n')

