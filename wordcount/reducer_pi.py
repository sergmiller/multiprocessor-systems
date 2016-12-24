#!/usr/bin/env python

import sys

last_key = None
running_total = 0

for input_line in sys.stdin:
   input_line = input_line.strip()
   this_key, this_part, value = input_line.split("\t", 1)
   this_key = int(this_key)
   this_part = int(this_part)
   value = int(value)

   if last_key == this_key:
       running_total += value
   else:
       if last_key:
           print( "%d\t%d" % (last_key, running_total) )
       running_total = value
       last_key = this_key

if last_key == this_key:
   print( "%d\t%d" % (last_key, running_total) )
