#!/usr/bin/env python

import sys
import re
import random

N = 100

def add_point(x,y):
    key = 2*(x>=0) + (y>=0)
    value = (x*x + y*y <= 1)
    print( "%d\t%d" % (key, value) )

for line in sys.stdin:
    continue
#    keys = line.split()
#    x,y = float(keys[0]), float(keys[1])
#    add_point(x,y)


for t in range(N):
    x,y = random.uniform(-1,1), random.uniform(-1,1)
    add_point(x,y)
