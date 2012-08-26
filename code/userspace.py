
'''  Copyright(C) 2012 Computer Science Capstone (Spring/Summer) Team
             2.718 Portland State University

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''

# userspace.py - A user space tool to read/display loki records
# 
# Currently supported version(s) of loki record file: 1
import sys
from struct import * # unpack instead of struct.unpack
import subprocess as sp #subprocess is used to call pahole
import re # regular expression support, provides search capabilities
import argparse #this gives a nice way to include command line arguments

theData = [] #a global to hold all the data

#===================ARGPARSE===================
	#first make a parser obj
parser = argparse.ArgumentParser(description="A tool to read loki generated Record files")

	#add arguments to the parser via parser.add_argument()
parser.add_argument("filename", help="A valid Loki record file",type=argparse.FileType('rb') )
parser.add_argument("-S", "--search", help="Search for items by name from the record file.",metavar="<string or regex>")
parser.add_argument("-V", "--version", help="Displays version information",action="version", version="%(prog)s 2.718")
parser.add_argument("-le", "--little-endian", help="Flag to display hex data in little endian", action="store_true")

	#parse arg[v] and put it into the args obj
args = parser.parse_args()
	#now args.argname can access various args, Example: recordFile = args.filename 

#Set big_endian based on system, used later on for printing based on -le flag
if pack("h",1) == "\000\001":
	big_endian = True
else:
	big_endian = False

#===================READ RECORRDS===================

def readFile():
	'''Reads the record file and returns the tuple (driver_name,dataItems) '''
	
	#record File is in args.filename
	recF = args.filename 

	#Read the;	magic number, record file version, size of driver name, 
	#			driver name, and recordCount from record file.
	try:
		magicNum, = unpack("4s",recF.read(4)) #magic number
		if not magicNum == "Loki":
			print "Not a loki record file!"
			exit(2)
			
		recFVersion, = unpack("B", recF.read(1)) #Version of recF format
		if recFVersion != 1: 
			print("Userspace tool is for version 1 record file only!")
			exit(2)
			
		namesize, = unpack("I", recF.read(4) ) #size in char of driver name
		driver_name,recordCount = unpack(("="+str(namesize)+"s"+" I"), recF.read(namesize+4)) #name, record count
	except OverflowError as e:
		print("%s" %e)
		print("The record may be corrupt")
		sys.exit(2)
	except Exception as e:
		print('Unspecified Error reading record file: %s' %e)
		sys.exit(2)
		
	
		
	#some tmp vars to hold pieces of each item
	name = ""
	namesize = 0
	itemSize = 0
	dataItems = []

	#for each record add a tuple to dataitems
	for i in range(recordCount):
		#get the size of name, the name, and size of data from each item
		try:
			namesize, = unpack("I", recF.read(4) )
			name,itemSize = unpack(("="+str(namesize)+"s"+" I"), recF.read(namesize+4))
		except OverflowError as e:
			print("%s" %e)
			print("Record file is either corrupt or lying about version")
			sys.exit(2)
		except Exception as e:
			print('Error reading record: %s' %e)
			sys.exit(2)

		data = recF.read(itemSize)
		dataItems.append( (name,itemSize,data))

	recF.close()
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
	map = get_map(name)
	
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
#===================PAHOLE===================	
def get_map(arg):
    map = {}
    lines = get_class(arg).split("\n")
    for line in lines:
        
        match = re.search("\t(.+[\w\d*])\s\s+(\w.*);.+/*\s+(\d+)\s+(\d+)",line)

        if match:

            t,name,offset,size =match.group(1),match.group(2),match.group(3),match.group(4)
            map[int(offset)] = (t,name,int(size))

    if map != {}:
        return map
    else:
        return None
            
def get_class(arg):
    return run_pahole(["-C",str(arg)])

def run_pahole(args=[]):
    #list of args passed in
    args =["pahole"]+args+["e1000.ko"]
    
    val=sp.check_output(args)
    return val
#===================PRINTING FUNCTIONS===================

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
			
		if data:

			## Print data in a more readable format.
			## -------------------------------------
			##

			## Add label to output string.
			##
			translatedData = tabs + "Value: "

			## Find all array dimension size values, if any.
			##
			dimensionSizes = re.findall("\[([0-9]+)\]", name)

			## (if 'data' represents an array)
			##
			if dimensionSizes:

				## Handles multi-dimensional arrays.
				##
				## This is a bit of python's functional programming -
				## the lambda function will perform multiplication
				## on successive pairs in the list of array dimension
				## sizes, which gives us the product of all elements
				## in the list.
				##
				numElements = reduce(lambda x, y : int(x) * int(y), dimensionSizes, 1)
				elementSize = size / numElements

				## Attempt to translate each array element.
				##
				for x in xrange(numElements):

					translatedData += translate(t, data[(x * elementSize):((x * elementSize) + elementSize)])
					translatedData += ' '

			## (if 'data' does not represent an array)
			##
			else:

				translatedData += translate(t, data)

			## Print final result.
			##
			print translatedData

#===================TRANSLATOR===================
## Provides modular implementation of type specific tranlations from binary
## format to readable format.
##
## David Huddleson
## 8/20/2012
## Last modified: 8/21/2012
##


## Declare global objects.
##
translators = {}


## Main function definition.
##
def translate(typeString, rawValue):

	if typeString in translators:

		return translators[typeString](rawValue)

	else:

		return defaultTranslator(rawValue)


## Helper function definition(s).
##
def addTranslator(typeString, translator):

	translators[typeString] = translator


## Translation function definition(s).
## -----------------------------------
##

## Default translator.
##
def defaultTranslator(rawValue):
	## Returns value in hex format: "0xAABBCCDDEEFF..."
	## Note: endianness is decided based on system native and -le flag
	##
	## args.little_endian: true if -le was used
	## big_endian: true if system is big endian (gets set in argparse section above) 

	# When these are both true then system is big endian, but -le is set
	# when they are both false then system is little endian, but -le is not set
	# in either case the endianness needs flipped
	if args.little_endian == big_endian:
		return "0x" + ''.join(["%02X" % ord(x) for x in reversed(rawValue)]).strip()
	else:
		# Otherwise, data is already in desired endianness
		return "0x" + ''.join(["%02X" % ord(x) for x in rawValue]).strip()



## Type-specific defintions.
## -------------------------
##


## Translator function for char type.
##
def char_translator(rawValue):

	## Filter out non-printable and whitespace ASCII characters.
	## We will only print ASCCI characters in the range (33 - 127):
	##
	## !"#$%&'()*+,-./0123456789:;<=>?@
	## ABCDEFGHIJKLMNOPQRSTUVWXYZ\]^_`
	## abcdefghijklmnopqrstuvwxyz{|}~
	##
	c = str(unpack('@c', rawValue)[0])
	
	if ord(c) > 32 and ord(c) < 127:

		## Return printable character.
		##
		return c

	else:
		## Return default byte translation.
		##
		return defaultTranslator(rawValue)

addTranslator("char", char_translator)


## Translator for signed char type.
##
def signed_char_translator(rawValue):

	return str(unpack('@b', rawValue)[0])

addTranslator("signed char", signed_char_translator)


## Translator for unsigned char type.
##
def unsigned_char_translator(rawValue):

	return str(unpack('@B', rawValue)[0])

addTranslator("unsigned char", unsigned_char_translator)


## Translator for short int type.
##
def short_int_translator(rawValue):

	return str(unpack('@h', rawValue)[0])

addTranslator("short int", short_int_translator)


## Translator for short unsigned int type.
##
def short_unsigned_int_translator(rawValue):

	return str(unpack('@H', rawValue)[0])

addTranslator("short unsigned int", short_unsigned_int_translator)


## Translator for int type.
##
def int_translator(rawValue):

	return str(unpack('@i', rawValue)[0])

addTranslator("int", int_translator)


## Translator for unsigned int type.
##
def unsigned_int_translator(rawValue):

	return str(unpack('@I', rawValue)[0])

addTranslator("unsigned int", unsigned_int_translator)


## Translator for long int type.
##
def long_int_translator(rawValue):

	return str(unpack('@l', rawValue)[0])

addTranslator("long int", long_int_translator)


## Translator for long unsigned int type.
##
def long_unsigned_int_translator(rawValue):

	return str(unpack('@L', rawValue)[0])

addTranslator("long unsigned int", long_unsigned_int_translator)


## Translator for long long int type.
##
def long_long_int_translator(rawValue):

	return str(unpack('@q', rawValue)[0])

addTranslator("long long int", long_long_int_translator)


## Translator for long long unsigned int type.
##
def long_long_unsigned_int_translator(rawValue):

	return str(unpack('@Q', rawValue)[0])

addTranslator("long long unsigned int", long_long_unsigned_int_translator)


## Translator for float type.
##
def float_translator(rawValue):

	return str(unpack('@f', rawValue)[0])

addTranslator("float", float_translator)


## Translator for double type.
##
def double_translator(rawValue):

	return str(unpack('@d', rawValue)[0])

addTranslator("double", double_translator)


## Translator for u8 type (unsigned byte value).
##
def u8_translator(rawValue):

	return str(unpack('=B', rawValue)[0])

addTranslator("u8", u8_translator)


## Translator for u16 type (unsigned 16-bit value).
##
def u16_translator(rawValue):

	return str(unpack('=H', rawValue)[0])

addTranslator("u16", u16_translator)


## Translator for u32 type (unsigned 32-bit value).
##
def u32_translator(rawValue):

	return str(unpack('=L', rawValue)[0])

addTranslator("u32", u32_translator)


## Translator for u64 type (unsigned 64-bit value).
##
def u64_translator(rawValue):

	return str(unpack('=Q', rawValue)[0])

addTranslator("u64", u64_translator)

## Translator for bool type.
##
def bool_translator(rawValue):
	
	return str(unpack('?',rawValue)[0])

addTranslator("bool",bool_translator)
#===================SEARCH FUNCTIONS===================
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
#===================RUN SOMETHING===================
driver_name, dataItems = readFile()
mapStructs(dataItems)
print("Driver name: " + driver_name)
if args.search:
	nameSearch(args.search)
else:
	printAll()










