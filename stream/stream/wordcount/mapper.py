#!/usr/bin/env python

import sys
import re

for line in sys.stdin:
	line = re.sub(r"[,.:;\"!?\\()=+]", ' ', line) 
	keys = line.split()
	pairs = [keys[i] + '#' + keys[i+1] for i in range(len(keys)-1)]	
	for p in pairs:
	print( "%s\t%d" % (p, 1)
