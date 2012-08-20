## bytetrans.py
## Provides modular implementation of type specific tranlations from binary
## format to readable format.
##
## David Huddleson
## 8/20/2012
## Last modified: 8/21/2012
##


## Import required modules.
##
import struct


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

	return ''.join(["%02X " % ord(x) for x in rawValue]).strip()


## Type-specific defintions.
## -------------------------
##


## Translator function for char type.
##
def char_translator(rawValue):

	return str(struct.unpack('=c', rawValue)[0])

addTranslator("char", char_translator)


## Translator for signed char type.
##
def signed_char_translator(rawValue):

	return str(struct.unpack('=b', rawValue)[0])

addTranslator("signed char", signed_char_translator)


## Translator for unsigned char type.
##
def unsigned_char_translator(rawValue):

	return str(struct.unpack('=B', rawValue)[0])

addTranslator("unsigned char", unsigned_char_translator)


## Translator for unsigned int type.
##
def unsigned_int_translator(rawValue):

	return str(struct.unpack('=I', rawValue)[0])

addTranslator("unsigned int", unsigned_int_translator)
