# -*- coding: utf-8

import sys
import os
import re
import binascii

def insert_slashx(string, every=2):
    return "\\x".join(string[i:i + every] for i in xrange(0, len(string), every))

def split_count(s, count):
     return ["".join(x) for x in zip(*[list(s[z::count]) for z in range(count)])]

var = "\\x" + insert_slashx(binascii.hexlify(open(sys.argv[1]).read()))
data = "echo -en \"" + "\" >> downloader\necho -en \"".join(split_count(var, (256))) + "\" >> downloader"
print data
