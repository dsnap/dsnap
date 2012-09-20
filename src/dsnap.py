#///////////////////////////////////////////////////////////////////////////////
#//
#//  dsnap - dsnap.py
#//
#//  This file is a user space tool that reads and displays driver snapshots.
#//  Supported version of dsnap record format: 1
#//
#///////////////////////////////////////////////////////////////////////////////

import sys
import argparse 		# Command line argument parsing.
import functools		# For python3 reduce() function.
import re 			# Regular expression support for search.
import subprocess as sp 	# Used to call pahole.

from struct import * 		# For use of unpack instead of struct.unpack.

pyversion = 2 if sys.version_info < (3, 0, 0) else 3
theData	= []



# == ARGUMENT PARSING ==========================================================

# Make a parser object.
parser = argparse.ArgumentParser(description = "A tool to read dsnap snapshots.")

# Add arguments to the parser.
parser.add_argument("filename", help = "A valid dsnap record file",
			type = argparse.FileType('rb'))

parser.add_argument("-S", "--search", help = "Search for items by name from " +
			"the record file.", metavar = "<string or regex>")

parser.add_argument("-V", "--version", help = "Displays version information",
			action = "version", version = "%(prog)s 2.718")

parser.add_argument("-l", "--little-endian", help = "Flag to display hex " +
			"data in little endian", action = "store_true")

args = parser.parse_args()

# Set big_endian based on system, used later on for printing based on -l flag.
if (pack("h", 1) == "\000\001"):
	big_endian = True
else:
	big_endian = False



# == RECORD READING ============================================================

'''
Reads the record file and returns a tuple.
@return: a data tuple (driver name and data items).
'''
def readFile():
	recF = args.filename 

	try:
		magicNum, = unpack("5s", recF.read(5))
		
		if pyversion == 3:
			magicNum = magicNum.decode("utf-8");

		if not magicNum == "dsnap":
			print("Not a dsnap record file!")
			exit(2)
			
		recFVersion, = unpack("B", recF.read(1))

		if recFVersion != 1: 
			print("Userspace tool is for version 1 record file "
				+ "only!")
			exit(2)
			
		namesize, = unpack("I", recF.read(4))	# Chars in driver name.
		driver_name, recordCount = unpack(("=" + str(namesize) + "s"
						+ " I"),
						recF.read(namesize + 4))

		if pyversion == 3:
			driver_name = driver_name.decode("utf-8")

	except OverflowError as e:
		print("%s" %e)
		print("The record may be corrupt")
		sys.exit(2)

	except Exception as e:
		print("Unspecified Error reading record file: %s" %e)
		sys.exit(2)
		
	name = ""
	namesize = 0
	itemSize = 0
	dataItems = []

	# For each record, add a tuple to dataitems.
	for i in range(recordCount):
		try:
			namesize, = unpack("I", recF.read(4))
			name, itemSize = unpack(("=" + str(namesize) + "s"
						+ " I"),
						recF.read(namesize + 4))

			if pyversion == 3:
				name = name.decode("utf-8")

		except OverflowError as e:
			print("%s" %e)
			print("Record file is either corrupt or lying about "
				+ "version")
			sys.exit(2)

		except Exception as e:
			print("Error reading record: %s" %e)
			sys.exit(2)

		data = recF.read(itemSize)
		dataItems.append((name, itemSize, data))

	recF.close()

	return (driver_name, dataItems)

'''
Arranges data into tuples 't' such that:
  - t = None -> The start of a new item from dataItems.
  - t = (None, name, None, None, None, level) -> The name of a new structure
  - t = (None, name, dsize, None, data, level) -> pahole unable to generate map
  - t = (t, name, size, p, None, level) -> data of this struct is also a struct 
  - t = (t, name, size, p, data[p:p + size], level) -> actual data item

@dataItems: the data to arrange into tuples.
'''
def mapStructs(dataItems):
	for item in dataItems:
		if (pyversion == 2):
			name = item[0].split(b'\0',1)[0]
		else:
			name = item[0].split('\0',1)[0]

		data = item[2]
		dsize = item[1]
		
		# Signifies the beginning of each data item.
		theData.append(None) 
		addStructs(name, data, dsize)

'''
Helper function for mapStructs. Recursively adds items to theData.
@name: TODO
@data: TODO
@dsize: TODO
@level: TODO
'''
def addStructs(name, data, dsize, level = 0):
	global theData
	map = get_map(name)
	
	if map:
		theData.append((None, name, None, None, None, level))
		for p in sorted(map.keys()):
			t,name,size = map[p]
			if "struct" in t and "*" not in t:
				theData.append((t, name, size, p, None, level))
				addStructs(t.split("struct ")[1],
							data[p:p + size],
							size,
							level + 1)
			else:
				theData.append((t, name, size, p,
						data[p:p + size], level))
	
	else:
		theData.append((None, name, dsize, None, data, level))

	return 



# == PAHOLE ====================================================================

'''
TODO
@arg: 
'''	
def get_map(arg):
	map = {}

	if pyversion == 2:
		lines = get_class(arg).split("\n")
	else:
		lines = get_class(arg).split(b"\n")

	for line in lines:
		if (pyversion == 3):
			line = line.decode("utf-8")
 
		match = re.search("\t(.+[\w\d*])\s\s+(\w.*);.+/*"
				+ "\s+(\d+)\s+(\d+)", line)

		if match:
			t, name, offset, size = match.group(1), match.group(2), match.group(3), match.group(4)

			map[int(offset)] = (t, name, int(size))

	if map != {}:
		return map
	else:
		return None

'''
TODO
@arg: 
'''
def get_class(arg):
    return run_pahole(["-C", str(arg)])

'''
TODO
@args:
'''
def run_pahole(args = []):
    args = ["pahole"] + args + ["e1000.ko"]
    val = sp.check_output(args)
    return val



# == PRINTING ==================================================================

'''
Prints all data items.
'''
def printAll():
	global theData
	for item in theData:
		if item:
			printItem(item)

'''
Prints a specific data item.
@item: the data item to print
'''
def printItem(item):
	t = item[0] 
	name = item[1]
	size = item[2]
	offset = item[3]
	data = item[4]
	header_indent = "\t" * item[5]
	item_indent = header_indent + "   "

	if not t and not size and not offset and not data:
		# Extend divider to 80 characters.
		print("\n" + header_indent + "========== " + name + " "
			+ ("=" * (80 - (item[5] * len("\t".expandtabs()))
			- len(name) - 2 - 10)))

	else:
		if not t:
			print("\n" + item_indent + "Name:\tUnknown " + name)
		else:
			print("\n" + item_indent + "Name:\t" + t + " " + name)
			
		print(item_indent + "Size:\t" + str(size))
		print(item_indent + "Offset:\t" + str(offset))
			
		if data:
			translatedData = item_indent + "Value:\t"

			## Find all array dimension size values, if any.
			dimensionSizes = re.findall("\[([0-9]+)\]", name)

			## Data represents an array.
			if dimensionSizes:
				'''
				Handles multi-dimensional arrays. The lambda
				function will perform multiplication on
				successive pairs in the list of array dimension
				sizes, which gives us the product of all
				elements in the list.
				'''
				if (pyversion == 2):
					numElements = reduce(lambda x, y :
								int(x) * int(y),
								dimensionSizes,
								1)
					
					elementSize = size / numElements

				else:
					numElements = functools.reduce(lambda x,
							y : int(x) * int(y),
							dimensionSizes,
							1)

					elementSize = int(size / numElements)

				# Translate each array element.
				if (pyversion == 2):
					for x in xrange(numElements):
						translatedData += translate(t,
							data[(x * elementSize):
							((x * elementSize)
							+ elementSize)]) + ' '

				else:
					for x in range(numElements):
						translatedData += translate(t,
							data[(x * elementSize):
							((x * elementSize)
							+ elementSize)]) + ' '
			
			# Data does not represent an array.
			else:
				translatedData += translate(t, data)

			# Print final result.
			print(translatedData)



# == TRANSLATORS ===============================================================

'''
Translators provide modular implementation of type-specific tranlations from
binary format to readable format.
'''
translateCodes = {"char" : "@c",
		"signed char" : "@b",
		"unsigned char" : "@B",
		"short int" : "@h",
		"short unsigned int" : "@H",
		"int" : "@i",
		"unsigned int" : "@I",
		"long int" : "@l",
		"long unsigned int" : "@L",
		"long long int" : "@q",
		"long long unsigned int" : "@Q",
		"float" : "@f",
		"double" : "@d",
		"u8" : "=B",
		"u16" : "=H",
		"u32" : "=L",
		"u64" : "=Q",	
		"bool" : "?"}

'''
Provides a modular implementation of type-specific translations from a binary
format to a more readable format.
@typeString: The type to translate to.
@rawValue: The value to translate.
'''
def translate(typeString, rawValue):
	if typeString == "char":
		return char_translator(rawValue)

	if typeString not in translateCodes:
		return defaultTranslator(rawValue)

	return str(unpack(translateCodes[typeString], rawValue)[0])

'''
Returns the raw value in hex format: "0xAABBCCDDEEFF...". Endianness is decided
based on system native and -l flag.

	args.little_endian: 	true if -l was used
	big_endian: 		true if system is big endian (set in argparse
				section above)

When both true, then system is big endian, but -l is set. When both are false,
then system is little endian, but -l is not set.
'''
def defaultTranslator(rawValue):
	# Need to change endianness.
	if args.little_endian == big_endian:
		if (pyversion == 2):
			return "0x" + ''.join(["%02X" % ord(x)
					for x in reversed(rawValue)]).strip()
		else:
			return "0x" + ''.join(["%02X" % x
					for x in reversed(rawValue)]).strip()

	# Data is already in desired endianness.
	else:
		if (pyversion == 2):
			return "0x" + ''.join(["%02X" % ord(x)
					for x in rawValue]).strip()
		else:
			return "0x" + ''.join(["%02X" % x
					for x in rawValue]).strip()

'''
Translator function for the char type.
@rawValue: The binary value to translate.
'''
def char_translator(rawValue):
	if (pyversion == 2):
		c = str(unpack(translateCodes["char"], rawValue)[0])
	else:
		c = str(unpack(translateCodes["char"],
			rawValue)[0].decode("utf-8"))

	# Return printable characters.
	if ord(c) > 32 and ord(c) < 127:
		return c

	# Return default byte translation.
	else:
		return defaultTranslator(rawValue)



# == SEARCH ====================================================================

'''
Searches for and prints data by the specified string or regular expression.
'''
def nameSearch(arg):
	found = 0

	for i,item in enumerate(theData):
		if not item:
			continue
			
		match = re.search(arg, item[1])
		
		if match:
			found = found + 1
			print("\n========== Item " + str(found) + " "
				+ ("=" * (80 - 17 - len(str(found)))))
			printItem(item)

			if not item[2]:
				printItem(theData[i + 1])

	print("\nFound " + str(found) + " items.\n")



# == EXECUTION =================================================================

driver_name, dataItems = readFile()
mapStructs(dataItems)

if args.search:
	nameSearch(args.search)
else:
	printAll()

