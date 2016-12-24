#!/usr/bin/env python

import sys
import re

for line in sys.stdin:
	line = re.sub(r"[,.:;\"!?\\()=+]", ' ', line) 
	keys = line.split()
	if keys[0] == '=====':
	    continue	
	print( "%s\t%d" % (keys[0], len(keys)-1) )
