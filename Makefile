#///////////////////////////////////////////////////////////////////////////////
#//
#//  Loki - Makefile
#//
#//  This Makefile builds Loki and the specified driver simultaneously by
#//  calling the individual makefiles of Loki and the driver in their
#//  their respective directories. This allows the diriver to access Loki's
#//  exported symbols.
#//
#///////////////////////////////////////////////////////////////////////////////

KERNEL_DIR = "/lib/modules/$(shell uname -r)/build"
SOURCE_DIR = "src"
DRIVER_DIR = "test-drivers"
LOKI = "loki"

obj-y := $(DRIVER_DIR)/$(DRIVER)/ $(SOURCE_DIR)/

all:
	$(shell cp -f $(SOURCE_DIR)/$(LOKI).h $(DRIVER_DIR)/$(DRIVER))
	make -C $(KERNEL_DIR) \
	M=$(PWD)
	
install:
	$(shell rmmod $(DRIVER))
	$(shell rmmod $(LOKI))
	$(shell insmod $(SOURCE_DIR)/$(LOKI).ko)
	$(shell insmod $(DRIVER_DIR)/$(DRIVER)/$(DRIVER).ko)
	$(shell cp -f $(DRIVER_DIR)/$(DRIVER)/$(DRIVER).ko $(SOURCE_DIR)/$(DRIVER).ko)
	
clean:
	make -C $(KERNEL_DIR) \
	M=$(PWD) \
	clean
