// TODO :
// 	Add functionality to read and write operations
// 	Implement the sorting function
// 	@Nitesh Meena
//

// Following macro makes each pr_info statement has a prifix of module name 

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");

enum{
	SORT_NONE,
	SORT_INT,
	SORT_STRING
};

struct process_entry{
	int pid;
	void * buffer;
	int type;
	int size;

	struct list_head node;
};

static void init_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL)
		return;

	entry_ptr->buffer = NULL;
	entry_ptr->size = 0;
	entry_ptr->type = SORT_NONE;
	entry_ptr->pid = current->pid;

	return;
}

static void clean_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL) return;

	if(entry_ptr->buffer) vfree(entry_ptr->buffer);

	return;
}


// TODO : 
// 	  Attach mutex locks to all the list operations
// 	  @Nitesh Meena
//

static struct list_head * process_list = NULL;
static struct semaphore * process_list_lock = NULL;

static struct proc_dir_entry * process_entry = NULL;


static struct process_entry * get_process_entry(int pid){
	
	struct process_entry * ret = NULL;
	struct process_entry * entry_ptr = NULL;
	struct list_head * list_ptr = NULL;

	pr_info("get_process_entry : enter\n");
	
	if(down_interruptible(process_list_lock)){
		return NULL;
	}

	list_for_each(list_ptr, process_list){
		entry_ptr = list_entry(list_ptr, struct process_entry, node);
		if(entry_ptr->pid == pid) ret = entry_ptr;
	}

	up(process_list_lock);
	
	pr_info("get_process_entry : return\n");

	return ret;
}


static int process_open_handler(struct inode * iptr, 
				struct file * fptr)
{
	struct process_entry * entry_ptr = NULL;

	pr_info("Open Function Called \n");
	pr_info("get_process_entry : entry\n");

	
	entry_ptr = get_process_entry(current->pid);
	if(entry_ptr){
		pr_info("One instance of file already open\n");
		return -1;
	}	
	
	entry_ptr = (struct process_entry * ) vmalloc(sizeof(struct process_entry));

	init_process_entry(entry_ptr);
	
	if(down_interruptible(process_list_lock)){
		clean_process_entry(entry_ptr);
		vfree(entry_ptr);
		return -ERESTARTSYS;
	}

	list_add(&entry_ptr->node, process_list);	
	
	up(process_list_lock);

	pr_info("get_process_entry : return\n");
	return 0;
}

static int process_close_handler(struct inode * iptr, 
				 struct file * fptr)
{

	struct process_entry * entry_ptr = NULL;

	pr_info("Close Function Called \n");
	pr_info("get_process_entry : entry\n");
	
	entry_ptr = get_process_entry(current->pid);
	
	if(entry_ptr == NULL)
		return -1;
	
	if(down_interruptible(process_list_lock)){
		return -ERESTARTSYS;
	}

	list_del(&entry_ptr->node);

	up(process_list_lock);
	
	clean_process_entry(entry_ptr);
	
	vfree(entry_ptr);	

	pr_info("get_process_entry : return\n");
	return 0;
}

static ssize_t process_read_handler(struct file * fptr, 
				    char __user * buffer, 
				    size_t size, 
				    loff_t * ppos)
{

	pr_info("Read Function Called \n");
	
		

	return 0;
}

static ssize_t process_write_handler(struct file * fptr, 
				     const char __user * buffer, 
				     size_t size, 
				     loff_t * ppos)
{

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

// TODO : 
// 	  This function will clean all the entries from process list
// 	  and avoid memory leaks  
// 	  @Nitesh Meena
// 	 
static void clean_process_list(void){
	return;
}

static __init int init_module_assign(void)
{
	pr_info("Creating Process : temp_proccess_entry\n");
	
	process_list_lock = (struct semaphore *) vmalloc(sizeof(struct semaphore));
	sema_init(process_list_lock, 1);

	process_list = (struct list_head *) vmalloc(sizeof(struct list_head));
	INIT_LIST_HEAD(process_list);	

	process_entry = proc_create("temp_process_entry", 0777, NULL, &process_operations);
	
	pr_info("Process Created\n");	
	return 0;
}

static __exit void exit_module_assign(void)
{
	pr_info("Removing Process : temp_process_entry\n");

	proc_remove(process_entry);

	clean_process_list();

	vfree(process_list);

	pr_info("Process Removed\n");
	return;
}


module_init(init_module_assign);
module_exit(exit_module_assign);
