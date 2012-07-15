# userspace.py - A rough draft implementation of the userspace tool in project Loki
# A proof of concept/rough draft by John Sackey for Team 2.718
#last update July 14 2012

#Used for handling binary data stored in files (specifically the blob)
from struct import *
#in order to have command line args import sys
import sys
import pahole

# ============BEGIN READING BLOB============================

# NOT SURE if we should leave a default blob location or not
# but hey THERE IT IS
# if a file name isn't provided, open the default blob location
if len(sys.argv) < 2:
	f = "/debug/e1000/blob.loki"
else:
	f = sys.argv[1]

# Try to open the blob, hopefully useful error if it fails
try:
	blob = open(f,'rb')
except Exception as e:
	print('Error opening the blob: %s' %e)
	sys.exit(2)	
		
#puts item count into blobcount the comma is because unpack returns tuple
blobcount, = unpack('I',blob.read(4))

#making sure I unpacked things right/made the test file correctly
#print blobcount

#some tmp vars to hold pieces of each item
name = ""
namesize = 0
itemSize = 0
dataItems = []

#for each item in the blob add a tuple to dataitems
for i in range(blobcount):
	#get the size of name, the name, and size of data from each item
	namesize, = unpack("I", blob.read(4) )
	#name, = unpack(str(namesize)+"s", blob.read(namesize))
	#itemSize, = unpack("I", blob.read(4))
	name,itemSize = unpack(("="+str(namesize)+"s"+" I"), blob.read(namesize+4))
	print(name)
	print(itemSize)
	data = blob.read(itemSize)
	dataItems.append( (name,itemSize,data))

blob.close()
#At this point dataItems is an array of tuples, and can be printed
#=================== END READING BLOB ============================

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
	
