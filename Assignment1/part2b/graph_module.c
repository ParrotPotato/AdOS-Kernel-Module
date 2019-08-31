// TODO :
// 	Remove the unnecessary printing statements and check more for the locking sitaation
//	@nitesh-meena

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

#define PB2_SET_TYPE 	_IOW(0x10, 0x31, int32_t*)
#define PB2_SET_ORDER 	_IOW(0x10, 0x32, int32_t*)
#define PB2_SET_INFO 	_IOW(0x10, 0x33, int32_t*)
#define PB2_SET_OBJ	_IOW(0x10, 0x34, int32_t*)

#define DATA_TYPE_NONE	 0x00
#define DATA_TYPE_INT	 0xff
#define DATA_TYPE_STRING 0xf0

#define GRAPH_IN_ORDER	 0x00
#define GRAPH_POST_ORDER 0x01
#define GRAPH_PRE_ORDER	 0x02

#define TRAV_LEFT_DONE 	 0x02
#define TRAV_RIGHT_DONE  0x04
#define TRAV_NODE_DONE	 0x08

struct obj_info{
	int32_t deg1cnt;
	int32_t deg2cnt;
	int32_t deg3cnt;

	int32_t maxdepth;
	int32_t mindepth;
};

struct search_obj{
	char objtype;
	char found;

	int32_t int_obj;
	char 	str[100];
	int32_t len;
};

struct graph_node{
	struct graph_node * right;
	struct graph_node * left;
	struct graph_node * parent;
	
	char type;

	int32_t int_value;
	char str[100];

	int len;

	int traverse;
};

//// GRAPH RELATED FUNCTIONS 

// TODO:
// 	tests add_node using inmodule function calls 
// 	- nitesh
//
struct graph_node * create_child_node(struct graph_node * parent)
{
	struct graph_node * node = (struct graph_node *) vmalloc(sizeof(struct graph_node));
	
	node->parent 	= parent;
	node->type	= DATA_TYPE_NONE;
	
	node->left 	= NULL;
	node->right	= NULL;
	node->traverse	= 0;

	node->len	= -1;

	return node;
}

struct graph_node * get_left_node(struct graph_node * parent)
{
	if(parent == NULL)
		return NULL;

	if(parent->left == NULL)
	{
		parent->left = create_child_node(parent);
	}

	return parent->left;
}

struct graph_node * get_right_node(struct graph_node * parent)
{
	
	if(parent == NULL)
		return NULL;

	if(parent->right == NULL)
	{
		parent->right = create_child_node(parent);
	}

	return parent->right;
}

void add_node(struct graph_node * root, void * buffer, char type )
{
	// Base case for the tree
	if(root->type == DATA_TYPE_NONE)
	{
		root->type = type;

		if((unsigned int)type == DATA_TYPE_INT)
		{
			root->int_value = *(int32_t *)(buffer);
		}
		else if((unsigned int)type == DATA_TYPE_STRING)
		{
			if(strlen((char *)buffer) > 100){
				// RETURN ERROR
			}

			strcpy(root->str, (char *)buffer);
			root->len = strlen(root->str);
		}
	}
	
	if((unsigned int)root->type == DATA_TYPE_STRING)
	{
		if(strcmp(root->str, (char *)buffer) > 0)
		{	
			add_node(get_right_node(root), buffer, type);
		}
		else
		{
			add_node(get_left_node(root), buffer, type);
		} 
	}
	else if((unsigned int)root->type == DATA_TYPE_INT)
	{
		if(*(int32_t *)(buffer) > root->int_value)
		{
			add_node(get_left_node(root), buffer, type);
		}
		else 
		{
			add_node(get_right_node(root), buffer, type);
		}
	}
}
// TODO:
// 	test search_graph using inmodule function calls
// 	- nitesh
struct graph_node * search_graph(struct graph_node * root, void * buffer)
{
	struct graph_node * current_node = root;
	char type = root->type ;

	while(current_node != NULL)
	{
		if((unsigned int)type == DATA_TYPE_INT)
		{
			if(current_node->int_value > *(int32_t *)(buffer))
			{
				current_node = current_node->left;
			}
			else if(current_node->int_value < *(int32_t *)(buffer))
			{
				current_node = current_node->right;
			}
			else
			{
				return current_node;
			}
		}
		else if((unsigned int)type == DATA_TYPE_STRING)
		{
			if((unsigned int)type == DATA_TYPE_INT)
			{
				if(strcmp(current_node->str, (char *) buffer) == 0)
				{
					return current_node;
				}
				else if(strcmp(current_node->str, (char *) buffer ) > 0)
				{
					current_node = current_node->left;
				}
				else
				{
					current_node = current_node->right;
				}
			}
		}	
	}

	return NULL;
}


// The following is the group of functions used for traversal 
// through the graph
// 	- nitesh

// TODO:
// 	test the traversal order using inmodule function calls
// 	- nitesh

int is_left_traversed(struct graph_node * node)
{
	return (node->traverse & TRAV_LEFT_DONE);
}

int is_right_traversed(struct graph_node * node)
{
	return (node->traverse & TRAV_RIGHT_DONE);
}

int is_node_traversed(struct graph_node * node)
{
	return (node->traverse & TRAV_NODE_DONE);
}

int is_node_done(struct graph_node * node)
{
	return ((node->traverse) & (TRAV_NODE_DONE | TRAV_RIGHT_DONE | TRAV_LEFT_DONE));
}

struct graph_node * in_order_traversal(struct graph_node * node)
{
	if(is_node_done(node))
	{
		node->traverse_graph = 0;
		return node->parent;
	}
	else if(!is_left_traversed(node))
	{
		node->traverse = node->traverse | TRAV_LEFT_DONE;
		return node->left;
	}
	else if(!is_node_traversed(node))
	{
		node->traverse = node->traverse | TRAV_NODE_DONE;
		return node;
	}
	else if(!is_right_traversed(node))
	{
		node->traverse = node->traverse | TRAV_RIGHT_DONE;
		return node->right;
	}
}

struct graph_node * pre_order_traversal(struct graph_node * node)
{
	if(is_node_done(node))
	{
		node->traverse_graph = 0;
		return node->parent;
	}
	else if(!is_node_traversed(node))
	{
		node->traverse = node->traverse | TRAV_NODE_DONE;
		return node;
	}
	else if(!is_left_traversed(node))
	{
		node->traverse = node->traverse | TRAV_LEFT_DONE;
		return node->left;
	}
	else if(!is_right_traversed(node))
	{
		node->traverse = node->traverse | TRAV_RIGHT_DONE;
		return node->right;
	}
}

struct graph_node * post_order_traversal(struct graph_node * node)
{
	
	if(is_node_done(node))
	{
		node->traverse_graph = 0;
		return node->parent;
	}
	else if(!is_left_traversed(node))
	{
		node->traverse = node->traverse | TRAV_LEFT_DONE;
		return node->left;
	}
	else if(!is_right_traversed(node))
	{
		node->traverse = node->traverse | TRAV_RIGHT_DONE;
		return node->right;
	}
	else if(!is_node_traversed(node))
	{
		node->traverse = node->traverse | TRAV_NODE_DONE;
		return node;
	}
}

struct graph_node * traverse_graph(struct graph_node * node, int mode)
{
	if(mode == GRAPH_IN_ORDER)
	{
		return in_order_traversal(node);
	}
	else if(mode == GRAPH_PRE_ORDER)
	{
		return pre_order_traversal(node);
	}
	else if(mode == GRAPH_POST_ORDER)
	{
		return post_order_traversal(node);
	}
	
	return NULL;
}

void delete_graph(struct graph_node * root)
{
	return;
}

//// ENDING



struct process_entry{
	
	int pid;
	
	int size;

	int write_count;
	int read_count;

	int type;
	
	struct graph_node * graph;
	
	struct list_head node;
};

static void init_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL)
		return;

	entry_ptr->graph 		= NULL;	
	
	entry_ptr->size 		= 0;
	entry_ptr->write_count 		= 0;
	entry_ptr->read_count 		= 0;
	
	entry_ptr->type 		= DATA_TYPE_NONE;
	entry_ptr->pid 			= current->pid;

	return;
}

static void clean_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL) return;

	delete_graph(entry_ptr->graph);
	
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
	
	return 0; 
}

static ssize_t process_write_handler(struct file * fptr, 
				     const char __user * buffer, 
				     size_t size, 
				     loff_t * ppos)
{
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
