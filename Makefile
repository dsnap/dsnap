#///////////////////////////////////////////////////////////////////////////////
#//
#//  dsnap - Makefile
#//
#//  This Makefile builds dsnap and the specified driver simultaneously by
#//  calling the individual makefiles of dsnap and the driver in their
#//  their respective directories. This allows the diriver to access dsnap's
#//  exported symbols.
#//
#///////////////////////////////////////////////////////////////////////////////

KERNEL_DIR = /lib/modules/$(shell uname -r)/build
SOURCE_DIR = src
DRIVER_DIR = test-drivers
DSNAP = dsnap
USAGE = "usage: make <target> DRIVER=<driver-name>"

obj-y := $(DRIVER_DIR)/$(DRIVER)/ $(SOURCE_DIR)/

all:
ifeq ($(DRIVER), )
	@echo $(USAGE)
else
	cp -f $(SOURCE_DIR)/$(DSNAP).h $(DRIVER_DIR)/$(DRIVER)
	make -C $(KERNEL_DIR) \
	M=$(PWD)
endif
	
install:
ifeq ($(DRIVER), )
	@echo $(USAGE)
else
	rmmod $(DRIVER)
	rmmod $(DSNAP)
	insmod $(SOURCE_DIR)/$(DSNAP).ko
	insmod $(DRIVER_DIR)/$(DRIVER)/$(DRIVER).ko
	cp -f $(DRIVER_DIR)/$(DRIVER)/$(DRIVER).ko $(SOURCE_DIR)/$(DRIVER).ko
endif
	
clean:
ifeq ($(DRIVER), )
	@echo $(USAGE)
else
	make -C $(KERNEL_DIR) \
	M=$(PWD) \
	clean
	rm -rf $(DRIVER_DIR)/$(DRIVER)/$(DSNAP).h
endif
