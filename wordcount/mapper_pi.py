#!/usr/bin/env python

import sys
import re
import random

N = 100

def add_point(x,y):
    key = (x*x + y*y <= 1)
    print( "%d\t%d\t%d" % (key, 2*(x>=0) + (y>=0), 1) )

for line in sys.stdin:
    keys = line.split()
	x,y = float(keys[0]), float(keys[1])
    key = (x*x + y*y <= 1)
	add_point(x,y)


# for t in range(N):
#     x,y = random.uniform(-1,1), random.uniform(-1,1)
#     add_point(x,y)
