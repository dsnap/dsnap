# userspace.py - A rough draft implementation of the userspace tool in project Loki
# A proof of concept/rough draft by John Sackey for Team 2.718

#Used for handling binary data stored in files (specifically the blob)
from struct import *
#in order to have command line args import sys
import sys
import pahole
"""
Now command line parameters can be grabbed from sys.argv
It is an array like in c so sys.argv[0] = script name
							sys.argv[1] = first arg
							...etc
One possible way to make a switch statement:
http://bytebaker.com/2008/11/03/switch-case-statement-in-python/
	
"""

#File obj named blob holds the test blob
blob = open("/debug/e1000/blob.loki", 'rb')

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

       return hexa

#making sure I unpacked things right/made the test file correctly
#print hex(blobIntro) #print format check in hex to see deadbeef!
#print blobcount

#some tmp vars to hold pieces of each item
name = ""
itemSize = 0
dataItems = []

#for each item in the blob add a tuple to dataitems
for i in range(blobcount):
	#get the name and size of data from each item
	name,itemSize = unpack("80s I", blob.read(80+4))
	data = blob.read(itemSize)
	dataItems.append( (name,itemSize,data))
	
#At this point dataItems is an array of tuples, and can be printed




def print_structs(name,data,level=0):
	tabs = "\t" *level
	map=pahole.get_map(name) 
	if map:
		print tabs+ "============"+name+"============"
		for p in sorted(map.keys()):
			t,name,size = map[p]
			print tabs+"Name:"+ t+" "+name
			print tabs+"Size:"+str(size)
			print tabs+"Offset" +str(p)
			if "struct" in t and "*" not in t:
				print_structs(t.split("struct ")[1],data[p:p+size],level+1)
			else:
				print tabs+"Value:"+ hexdump(data[p:p+size],int(size))
		return True
	return False



#As just a list of names
for item in dataItems:

	name = item[0].split(b'\0',1)[0]
	map=None
	data = item[2]
	dsize = item[1]
	print "*" *80 
	if not print_structs(name,data):
		print "============"+name+"============"
		print hexdump(item[2],16)+"\n"
	
#Just the raw data




# for item in dataItems:
# 	


# blobEnd, = unpack('I',blob.read(4))
# #print hex(blobEnd)
# blob.close()
