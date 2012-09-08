obj-y := test-driver/e1000/ src/

all:
	$(shell cp -f src/loki.h test-driver/e1000)
	make -C /lib/modules/$(shell uname -r)/build \
	M=$(PWD)
	
load:
	$(shell rmmod e1000)
	$(shell rmmod loki)
	$(shell insmod src/loki.ko)
	$(shell insmod test-driver/e1000/e1000.ko)
	$(shell cp -f test-driver/e1000/e1000.ko src/e1000.ko)
	
clean:
	make -C /lib/modules/$(shell uname -r)/build \
	M=$(PWD) \
	clean
