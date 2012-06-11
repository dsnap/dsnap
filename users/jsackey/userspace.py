#!/usr/bin/python2
# userspace.py - A rough draft implementation of the userspace tool in project Loki
# A proof of concept/rough draft by John Sackey for Team 2.718

#Used for handling binary data stored in files (specifically the blob)
from struct import *
#in order to have command line args import sys
import sys
"""
Now command line parameters can be grabbed from sys.argv
It is an array like in c so sys.argv[0] = script name
							sys.argv[1] = first arg
							...etc
One possible way to make a switch statement:
http://bytebaker.com/2008/11/03/switch-case-statement-in-python/
	
"""

#File obj named blob holds the test blob
#blob = open("test.blob", 'rb')
blob = open("/debug/e1000/blob.loki",'rb')
#puts format check into blobIntro and item count into blobcount
blobIntro,blobcount = unpack('II',blob.read(8))

#Function from: code.activestate.com/recipes/142812-hex-dumper/
#What it does is dumps the data contents in a pretty 3 column view
#and gives a nice starting point to create functions to only show hex
#or ascii
def hexdump(src, length=8):
    result = []
    digits = 4 if isinstance(src, unicode) else 2
    for i in xrange(0, len(src), length):
       s = src[i:i+length]
       hexa = b' '.join(["%0*X" % (digits, ord(x))  for x in s])
       text = b''.join([x if 0x20 <= ord(x) < 0x7F else b'.'  for x in s])
       result.append( b"%04X   %-*s   %s" % (i, length*(digits + 1), hexa, text) )
    return b'\n'.join(result)


#making sure I unpacked things right/made the test file correctly
print hex(blobIntro) #print format check in hex to see deadbeef!
print blobcount

#some tmp vars to hold pieces of each item
name = ""
itemSize = 0
dataItems = []

#for each item in the blob add a tuple to dataitems
for i in range(blobcount):
	#get the name and size of data from each item
	name,itemSize = unpack("80s I", blob.read(80+4))
	data = blob.read(itemSize)
	dataItems.append( (name.split(b'\0',1)[0],itemSize,data))
	
#At this point dataItems is an array of tuples, and can be printed


#As name/data pairs...	
"""
print "Name:      | Size: | Data: \n"
i = 0
for item in dataItems:
	listtest = splitString(item[2],20)
	liststr = "| "+listtest[0]+"\n"
	liststr1 = ""
	for x in range(1,len(listtest)):
		liststr1 = liststr1 + "                   | "+listtest[x]+"\n"
	print item[0] + "  | " + str(item[1]) + "    " + liststr + liststr1
"""
"""
for item in dataItems:
	print "name:"+item[0] +"\ndata:\n"+ item[2]

#As just a list of names
for item in dataItems:
	print "name:"+item[0]
	
#Just the raw data
for item in dataItems:
	print "Data:\n"+item[2]

#or size
for item in dataItems:
	print "Data size of "+item[0]+": "+str(item[1])+" Bytes\n"
"""

print "OFFSET                         HEX                              ASCII\n"
for item in dataItems:
	print item[0],item[1]
	print hexdump(item[2],16)+"\n"


blobEnd, = unpack('I',blob.read(4))
#print hex(blobEnd)
blob.close()

