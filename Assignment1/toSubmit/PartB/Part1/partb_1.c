/*
 * Name and Roll Nos :
 *      Nitesh Meena (16CS30023)
 *      G Rahul Kranti Kiran (16CS10018)
 *
 * Kernel Version : 
 *      5.2.9 
 */

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
#include <linux/string.h>

#define SORT_TYPE_NONE	 0x00
#define SORT_TYPE_INT	 0xff
#define SORT_TYPE_STRING 0xf0

MODULE_LICENSE("GPL");

struct process_entry{
	int pid;
	
	int * int_buffer;
	char ** string_buffer;
	int type;
	int size;
	
	int write_count;

	int read_count;

	struct list_head node;
};

static void init_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL)
		return;

	entry_ptr->int_buffer		= NULL;
	entry_ptr->string_buffer	= NULL;
	
	entry_ptr->size 		= 0;
	entry_ptr->write_count 		= 0;
	entry_ptr->read_count 		= 0;
	
	entry_ptr->type 		= SORT_TYPE_NONE;
	entry_ptr->pid 			= current->pid;

	return;
}

static void clean_process_entry(struct process_entry * entry_ptr){
	int i = 0 ;
	if(entry_ptr == NULL) return;

	if(entry_ptr->int_buffer) vfree(entry_ptr->int_buffer);
	if(entry_ptr->string_buffer)
	{
		for(i=0 ; i < entry_ptr->size ; i++){
			vfree(entry_ptr->string_buffer[i]);
		}
		vfree(entry_ptr->string_buffer);
	}	

	return;
}


static struct list_head * process_list = NULL;
static struct semaphore * process_list_lock = NULL;

static struct proc_dir_entry * process_entry = NULL;


static struct process_entry * get_process_entry(int pid){
	
	struct process_entry * ret = NULL;
	struct process_entry * entry_ptr = NULL;
	struct list_head * list_ptr = NULL;

	
	if(down_interruptible(process_list_lock)){
		return NULL;
	}

	list_for_each(list_ptr, process_list){
		entry_ptr = list_entry(list_ptr, struct process_entry, node);
		if(entry_ptr->pid == pid) ret = entry_ptr;
	}

	up(process_list_lock);
	

	return ret;
}

// simple implementation of bubble sort
static int sort_int_array(int32_t arr[], int n){
	int i= 0,j = 0; 
	
	for( i = 0 ; i < n ; i++)
	{
		for(j = i + 1  ; j < n ; j++){
			if(arr[i] > arr[j]){
				arr[j] = arr[i] + arr[j];
				arr[i] = arr[j] - arr[i];
				arr[j] = arr[j] - arr[i];
			}
		}
	}

	return 0;
}

// simple implementation of bubble sort
static int sort_string_array(char * arr[], int n){
	int i = 0 , j = 0 ;
	char string_temp[100];	
	for( i = 0 ; i < n ; i++)
	{
		for(j = i + 1  ; j < n ; j++){
			if(strcmp(arr[i], arr[j]) > 0){
				memcpy(string_temp, arr[i], 100);
				memcpy(arr[i], arr[j], 100);
				memcpy(arr[j], string_temp, 100);	
			}
		}
	}

	return 0;
}


static int process_open_handler(struct inode * iptr, 
				struct file * fptr)
{
	struct process_entry * entry_ptr = NULL;

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

	pr_info("Adding process entry for pid %d\n", current->pid);
	list_add(&entry_ptr->node, process_list);	
	
	up(process_list_lock);

	return 0;
}

static int process_close_handler(struct inode * iptr, 
				 struct file * fptr)
{

	struct process_entry * entry_ptr = NULL;

	
	entry_ptr = get_process_entry(current->pid);
	
	if(entry_ptr == NULL)
		return -1;
	
	if(down_interruptible(process_list_lock)){
		return -ERESTARTSYS;
	}

	pr_info("Removing process entry for pid %d\n", current->pid);

	list_del(&entry_ptr->node);

	up(process_list_lock);
	
	clean_process_entry(entry_ptr);
	
	vfree(entry_ptr);	

	return 0;
}

static ssize_t process_read_handler(struct file * fptr, 
				    char __user * buffer, 
				    size_t size, 
				    loff_t * ppos)
{
	struct process_entry * entry_ptr = NULL;
	int ret = 0 ;	
	entry_ptr = get_process_entry(current->pid);
	
	if(entry_ptr == NULL) return -EINVAL;

	if(entry_ptr->type == SORT_TYPE_NONE || entry_ptr->size == 0 || entry_ptr->write_count != entry_ptr->size){
		return -EACCES;
	}
	
	if(entry_ptr->read_count < entry_ptr->size)
	{
		if(entry_ptr->type == SORT_TYPE_INT)
		{
			memcpy(buffer, &entry_ptr->int_buffer[entry_ptr->read_count], sizeof(int32_t));
			entry_ptr->read_count += 1;
			return sizeof(int32_t);
		}
		if(entry_ptr->type == SORT_TYPE_STRING)
		{
			if(size < 100) 
			{
				memcpy(buffer, entry_ptr->string_buffer[entry_ptr->read_count], size);
			}
			else {
				memcpy(buffer, entry_ptr->string_buffer[entry_ptr->read_count], 100);
			}	
			ret = strlen(entry_ptr->string_buffer[entry_ptr->read_count]);
			entry_ptr->read_count += 1;
			return ret; 
		}
	}

	return -EACCES;
}



static ssize_t process_write_handler(struct file * fptr, 
				     const char __user * buffer, 
				     size_t size, 
				     loff_t * ppos)
{
	struct process_entry * entry_ptr = NULL;

	entry_ptr = get_process_entry(current->pid);

	if(entry_ptr == NULL){
		return -EINVAL;	
	}

	if(entry_ptr->type == SORT_TYPE_NONE){
	// Getting the first 2 bytes of the for the data descriptor 

		if(size != 2)
		{
			// return error code specific to this

		       	pr_info("Error Occured : Invalid size of %lu\n", size);
			return -EINVAL;
		}

		if((unsigned char)buffer[0] == SORT_TYPE_INT)
		{
			entry_ptr->type = SORT_TYPE_INT;
			entry_ptr->size = (int)buffer[1];

			entry_ptr->int_buffer = (int *) vmalloc(sizeof(int32_t) * entry_ptr->size);
		}
		else if((unsigned char)buffer[0] == SORT_TYPE_STRING)
		{
			entry_ptr->type = SORT_TYPE_STRING;
			entry_ptr->size = buffer[1];
			
			entry_ptr->string_buffer = (char **) vmalloc(sizeof(char **) * entry_ptr->size);
		}
		else
		{
			// return error code specific to this
		       	pr_info("Error Occured : Invalid sorting option : (int:%d - SORT_TYPE_INT : %d - your given type) \n", SORT_TYPE_INT, buffer[0]);	
			return -EINVAL;
		}
	}
	else{
	// Getting the actual array/string in the buffer 

		if(entry_ptr->type == SORT_TYPE_INT && entry_ptr->write_count  < entry_ptr->size)
		{
			memcpy(&entry_ptr->int_buffer[entry_ptr->write_count], buffer, sizeof(int32_t));
			entry_ptr->write_count += 1 ;

			if(entry_ptr->size == entry_ptr->write_count)
			{
				sort_int_array(entry_ptr->int_buffer, entry_ptr->size);
			}
		}
		
		else if(entry_ptr->type == SORT_TYPE_STRING && entry_ptr->write_count < entry_ptr->size && size < 100 && size > 1)
		{
			if(size < 0 || size > 100) return -EINVAL;

			entry_ptr->string_buffer[entry_ptr->write_count] = (char *) vmalloc(100);
			memset(entry_ptr->string_buffer[entry_ptr->write_count], 0, 100);
			

			memcpy(entry_ptr->string_buffer[entry_ptr->write_count], buffer, size);

			entry_ptr->write_count += 1 ;
			if(entry_ptr->size == entry_ptr->write_count)
			{
				sort_string_array(entry_ptr->string_buffer, entry_ptr->size);
			}
			
		}

		else
		{
			return -EINVAL;
		}
	}
	
	return entry_ptr->size - entry_ptr->write_count;
} 

static struct file_operations process_operations = 
{
	.owner 	= THIS_MODULE,
	.open	= process_open_handler,
	.read	= process_read_handler,
	.write	= process_write_handler,
	.release= process_close_handler,
};

static void clean_process_list(void){
	
	struct process_entry * process_ptr = NULL;
	struct list_head * list_ptr 	   = NULL;

	while(!list_empty(process_list))
	{
		list_ptr = process_list->next;
		process_ptr = list_entry(list_ptr, struct process_entry, node);

		list_del(list_ptr);
		clean_process_entry(process_ptr);

		vfree(process_ptr);
	}
}

static __init int init_module_assign(void)
{
	pr_info("Creating Process : partb_1_16CS30023_16CS10018\n");
	
	process_list_lock = (struct semaphore *) vmalloc(sizeof(struct semaphore));
	sema_init(process_list_lock, 1);

	process_list = (struct list_head *) vmalloc(sizeof(struct list_head));
	INIT_LIST_HEAD(process_list);	

	process_entry = proc_create("partb_1_16CS30023_16CS10018", 0777, NULL, &process_operations);
	
	return 0;
}

static __exit void exit_module_assign(void)
{
	pr_info("Removing Process : partb_1_16CS30023_16CS10018\n");

	proc_remove(process_entry);

	clean_process_list();

	vfree(process_list);

	return;
}


module_init(init_module_assign);
module_exit(exit_module_assign);
