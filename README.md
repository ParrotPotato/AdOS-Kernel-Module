### Making a kernel module

Building a kernel module should be really easy as a Makefile is provided along with the module code. Simply open a command shell in the corresponding directory and type `make`. The will generate many files in the directory (if everything was altight with the code and the dependencies), but the file we are concerned with is .ko extension: That is our kernel module binary. 

To load the binary simply type:

`sudo insmod <module_name>.ko   [argument1_name = argument1_value] ... `

This will load the kernel module (or give error is something is not right).

To remove the kernel module type:

`sude rmmod <module_name>.ko`

This should do the trick.


