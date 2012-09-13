#///////////////////////////////////////////////////////////////////////////////#//
#//  dsnap - Makfile
#//
#//  This is the makefile that builds the dsnap module.
#//
#///////////////////////////////////////////////////////////////////////////////

obj-m += dsnap.o

all:
	make -C /lib/modules/$(shell uname -r)/build \
	M=$(PWD) \
	KBUILD_EXTRA_SYMBOLS=$(PWD)/Module.symvers \
	modules

clean:
	make -C /lib/modules/$(shell uname -r)/build \
	M=$(PWD) \
	clean
