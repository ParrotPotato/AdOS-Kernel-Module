obj-m += partb_1.o

all:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) clean 
