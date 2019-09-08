obj-m += partb_2.o

all:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) clean 

insert: all
	sudo insmod partb_2.ko

update: all
	sudo rmmod partb_2
	sudo insmod partb_2.ko

remove:
	sudo rmmod partb_2

