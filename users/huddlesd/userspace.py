#newser.py - A new user space tool to read/display loki blobs
#			 rewritten to be more modular and extendable.
import sys
from struct import * #lets me use unpack instead of struct.unpack
import pahole
import re #regular expression support, gives us simple search
import argparse #this gives a nice way to parse arguments
import bytetrans

theData = [] #a global to hold all the data

#===================SET UP ARGPARSE=============================
	#first make a parser obj
parser = argparse.ArgumentParser(description="A tool to read loki generated blob files")

	#add arguments to the parser via parser.add_argument()
parser.add_argument("filename", help="A valid Loki Blob file",type=argparse.FileType('rb') )
parser.add_argument("-S", "--search", help="Search for items by name from the blob file.",metavar="<string or regex>")
parser.add_argument("-V", "--version", help="Displays version information",action="version", version="%(prog)s 2.718")
	
	#parse arg[v] and put it into the args obj
args = parser.parse_args()
	#now args.argname can access various args, Example: blob = args.filename 
#print vars(args)
#print args

#====================================================
#open specified blob and extract data from file obj
def readBlob():
	'''Reads the blob file and returns the tuple (driver_name,dataItems) '''

	blob = args.filename

	#puts item count into blobcount the comma is because unpack returns tuple
	try:
		namesize, = unpack("I", blob.read(4) )
		driver_name,blobcount = unpack(("="+str(namesize)+"s"+" I"), blob.read(namesize+4))
	except OverflowError as e:
		print("%s" %e)
		print("Possible blob corruption, not a blob, or format changed without userspace consent!")
		sys.exit(2)
	except Exception as e:
		print('Unspecified Error reading blob: %s' %e)
		sys.exit(2)
	
	#some tmp vars to hold pieces of each item
	name = ""
	namesize = 0
	itemSize = 0
	dataItems = []

	#for each item in the blob add a tuple to dataitems
	for i in range(blobcount):
		#get the size of name, the name, and size of data from each item
		try:
			namesize, = unpack("I", blob.read(4) )
			name,itemSize = unpack(("="+str(namesize)+"s"+" I"), blob.read(namesize+4))
		except OverflowError as e:
			print("%s" %e)
			print("Is the blob a blob is it formatted wrong is it corrupt <<MAKE THIS NICER")
			sys.exit(2)
		except Exception as e:
			print('Error reading blob: %s' %e)
			sys.exit(2)

		data = blob.read(itemSize)
		dataItems.append( (name,itemSize,data))

	blob.close()
	return (driver_name,dataItems)	
'''
After calling mapStructs theData is a list of tuples t such that:
	- t = None ->  the start of a new item from dataItems
	- t = (None, name, None, None, None, level) -> the name of a new structure
	- t = (None,name,dsize,None,data,level) -> pahole.py was unable to generate a map
	- t = (t, name, size, p, None, level) -> data of this structure is itself a struct 
	- t = (t,name,size,p,data[p:p+size],level) -> actual data item'''
def mapStructs(dataItems):
	for item in dataItems:
		name = item[0].split(b'\0',1)[0]
		data = item[2]
		dsize = item[1]
		#my ghetto way of signifying the beginning of each dataItem
		theData.append(None) 
		addStructs(name,data,dsize)		

	return

#Helper function for mapStructs, recursively adds items to theData
def addStructs(name, data,dsize,level=0):
	global theData
	map = pahole.get_map(name)
	
	if map:
		theData.append((None,name,None,None,None,level))
		for p in sorted(map.keys()):
			t,name,size = map[p]
			if "struct" in t and "*" not in t:
				theData.append( (t, name, size, p, None, level))
				addStructs(t.split("struct ")[1],data[p:p+size],size,level+1)
			else:
				theData.append( (t,name,size,p,data[p:p+size],level) )
	else:
		theData.append( (None,name,dsize,None,data,level) )
	return 

#=============================PRINTING FUNCTIONS=======================

# a print all function, usefull later? Maybe, for now it's mainly a tool 
# for making sure the other stuff is working correctly
# prints everything in the global var 'theData' simulates original 
# userspace output
def printAll():
	global theData
	for item in theData:
		if not item:
			print "*"*80
			continue
		printItem(item)

def printItem(item):
	t = item[0] 
	name = item[1]
	size = item[2]
	offset = item[3]
	data = item[4]
	tabs = "\t" * item[5]
	if not t and not size and not offset and not data:
		print tabs+ "============"+name+"============"
	else:
		if not t:
			print tabs+"Name: Unknown "+name
		else:
			print tabs+"Name: "+t+" "+name
			
		print tabs+"Size: "+str(size)
		
		print tabs+"Offset: "+str(offset)
			
		if not data:
			pass
		else:
			print tabs+"Value: "+bytetrans.translate(t, data)

#======================SEARCH FUNCTIONS=================================
def nameSearch(arg):
	found = 0
	for i,item in enumerate(theData):
		if not item:
			continue
			
		match = re.search(arg,item[1])
		if match:
			found = found+1
			print "=========Item "+str(found)+" ============"
			printItem(item)
			if not item[2]:
				printItem(theData[i+1])
	print "Found "+str(found)+" items"
#==================================RUN SOMETHING========================
driver_name, dataItems = readBlob()
mapStructs(dataItems)
print("Driver name: " + driver_name)
if args.search:
	nameSearch(args.search)
else:
	printAll()










