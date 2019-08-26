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
#include <linux/string.h>

MODULE_LICENSE("GPL");

enum{
	SORT_NONE,
	SORT_INT,
	SORT_STRING
};

struct process_entry{
	int pid;
	
	int * int_buffer;
	char ** string_buffer;
	int type;
	int size;
	
	int element_recieved;

	struct list_head node;
};

static void init_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL)
		return;

	entry_ptr->int_buffer		= NULL;
	entry_ptr->string_buffer	= NULL;
	entry_ptr->size 		= 0;
	entry_ptr->element_recieved 	= 0 ;
	entry_ptr->type 		= SORT_NONE;
	entry_ptr->pid 			= current->pid;

	return;
}

static void clean_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL) return;

	if(entry_ptr->int_buffer) vfree(entry_ptr->int_buffer);
	if(entry_ptr->string_buffer) vfree(entry_ptr->string_buffer);

	return;
}


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

// simple implementation of bubble sort
static int sort_int_array(int arr[], int n){
	int i= 0,j = 0; 
	
	for( i = 0 ; i < n ; i++)
	{
		for(j = i + 1  ; j < n ; j++){
			if(arr[i] < arr[j]){
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
	char * string_ptr = 0 ;	
	
	for( i = 0 ; i < n ; i++)
	{
		for(j = i + 1  ; j < n ; j++){
			if(strcmp(arr[i], arr[j]) > 0){
				string_ptr = arr[i];
				arr[i] = arr[j];
				arr[j] = string_ptr;
			}
		}
	}

	return 0;
}


static int process_open_handler(struct inode * iptr, 
				struct file * fptr)
{
	struct process_entry * entry_ptr = NULL;

	pr_info("Open Function Called \n");

	
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

	return 0;
}

static int process_close_handler(struct inode * iptr, 
				 struct file * fptr)
{

	struct process_entry * entry_ptr = NULL;

	pr_info("Close Function Called \n");
	
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
	struct process_entry * entry_ptr = NULL;

	pr_info("Write Function Called \n");

	entry_ptr = get_process_entry(current->pid);

	if(entry_ptr == NULL){
		return -EINVAL;	
	}

	if(entry_ptr->type == SORT_NONE){
	// Getting the first 2 bytes of the for the data descriptor 

		pr_info("Initializing Sorting Data\n");

		if(size != 2)
		{
			// return error code specific to this

		       	pr_info("Error Occured");
			return -EINVAL;
		}

		if(buffer[0] == 0xff)
		{
			entry_ptr->type = SORT_INT;
			entry_ptr->size = buffer[1];

			entry_ptr->int_buffer = (int *) vmalloc(sizeof(int32_t) * entry_ptr->size);
		}
		else if(buffer[0] == 0xf0)
		{
			entry_ptr->type = SORT_STRING;
			entry_ptr->size = buffer[1];
			
			entry_ptr->string_buffer = (char **) vmalloc(sizeof(char *) * entry_ptr->size);
		}
		else
		{
			// return error code specific to this
		       	pr_info("Error Occured");	
			return -EINVAL;
		}
	}
	else{
	// Getting the actual array/string in the buffer 
		pr_info("Reading data\n");

		if(entry_ptr->type == SORT_INT && entry_ptr->element_recieved  < entry_ptr->size)
		{
			entry_ptr->int_buffer[entry_ptr->element_recieved] = buffer[0];
			entry_ptr->element_recieved += 1 ;

			if(entry_ptr->size == entry_ptr->element_recieved){
				sort_int_array(entry_ptr->int_buffer, entry_ptr->size);

				// @DEBUG : 
				int i= 0 ;

				printk(KERN_INFO "%d", entry_ptr->int_buffer[0]);
				for (i=1 ; i < entry_ptr->size ; i++){
					printk(KERN_CONT "%d", entry_ptr->int_buffer[i]);
				}

				printk("\n");
			}


		}
		
		else if(entry_ptr->type == SORT_STRING && entry_ptr->element_recieved < entry_ptr->size)
		{
			if(size < 0 || size > 100) return -EINVAL;

			entry_ptr->string_buffer[entry_ptr->element_recieved] = (char *) vmalloc(sizeof(char) * size);
			strcpy(entry_ptr->string_buffer[entry_ptr->element_recieved], buffer);

			entry_ptr->element_recieved += 1 ;

			if(entry_ptr->size == entry_ptr->element_recieved){
				sort_string_array(entry_ptr->string_buffer, entry_ptr->size);
			}
			
		}
	}
	
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
