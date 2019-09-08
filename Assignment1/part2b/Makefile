obj-m += graph_module.o

all:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) clean 

insert: all
	sudo insmod graph_module.ko

update: all
	sudo rmmod graph_module
	sudo insmod graph_module.ko

remove:
	sudo rmmod graph_module

test: 
	gcc  tester.c -o tester
	./tester
