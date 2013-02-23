#!/usr/bin/env python
import commands
import os

path = "/home/trinity/src/kern/conf"
os.chdir(path)
print os.getcwd()
status, output = commands.getstatusoutput("./config ASST1") 
print status
print output
path = "/home/trinity/src/kern/compile/ASST1/"
os.chdir(path)
print os.getcwd()
status, text = commands.getstatusoutput('bmake depend')
print status
print text
status, text = commands.getstatusoutput('bmake')
print status
print text
status, text = commands.getstatusoutput('bmake install')
print status
print text
