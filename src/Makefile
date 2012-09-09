#///////////////////////////////////////////////////////////////////////////////#//
#//  Loki - Makfile
#//
#//  This is the makefile that builds the Loki module.
#//
#///////////////////////////////////////////////////////////////////////////////

obj-m += loki.o

all:
	make -C /lib/modules/$(shell uname -r)/build \
	M=$(PWD) \
	KBUILD_EXTRA_SYMBOLS=$(PWD)/Module.symvers \
	modules

clean:
	make -C /lib/modules/$(shell uname -r)/build \
	M=$(PWD) \
	clean
