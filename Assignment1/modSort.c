#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/limits.h>

// Pointers to global structures

static struct proc_dir_entry * process_entry;

MODULE_LICENSE("GPL");

static int process_open_handler(struct inode * iptr, struct file * fptr){
	pr_info("Open Function Called \n");
	return 0;
}

static int process_close_handler(struct inode * iptr, struct file * fptr){
	pr_info("Close Function Called \n");
	return 0;
}

static ssize_t process_read_handler(struct file * fptr, char __user * buffer, size_t size, loff_t * ppos)
{
	pr_info("Read Function Called \n");
	return 0;
}

static ssize_t process_write_handler(struct file * fptr, const char __user * buffer, size_t size, loff_t * ppos){
	pr_info("Write Function Called \n");
	return size;
} 

static struct file_operations process_operations = 
{
	.owner 	= THIS_MODULE,
	.open	= process_open_handler,
	.read	= process_read_handler,
	.write	= process_write_handler,
	.release= process_close_handler,
};


static __init int init_module_assign(void)
{
	pr_info("Creating Process\n");
	
	process_entry = proc_create("temp_process_entry", 0777, NULL, &process_operations);
	
	pr_info("Process Created\n");	
	return 0;
}

static __exit void exit_module_assign(void)
{
	pr_info("Removing Process\n");

	proc_remove(process_entry);

	pr_info("Process Removed\n");
	return;
}


module_init(init_module_assign);
module_exit(exit_module_assign);
