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

#define PB2_SET_TYPE 	 _IOW(0x10, 0x31, int32_t*)
#define PB2_SET_ORDER 	 _IOW(0x10, 0x32, int32_t*)
#define PB2_SET_INFO 	 _IOW(0x10, 0x33, int32_t*)
#define PB2_SET_OBJ	 _IOW(0x10, 0x34, int32_t*)

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

	int travers;
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
	node->travers	= 0;

	node->int_value = 0;
	memset(node->str, 0, 100);

	node->len	= -1;

	return node;
}

struct graph_node * create_root_node(void * buffer, char type)
{
	struct graph_node * node = (struct graph_node *) vmalloc(sizeof(struct graph_node));


	node->parent = NULL;
	node->type = DATA_TYPE_INT;

	node->left = NULL;
	node->right = NULL;
	node->travers = 0;

	node->len = -1;
	
	node->int_value = 0;
	memset(node->str, 0, 100);

	if((unsigned char) type == DATA_TYPE_INT)
	{
		node->int_value = * (int32_t *) buffer;
	}
	else if((unsigned char) type == DATA_TYPE_STRING)
	{
		strcpy(node->str, (char *)buffer);
		node->len = strlen((char *) buffer);
	}

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

		if((unsigned char)type == DATA_TYPE_INT)
		{
			root->int_value = *(int32_t *)(buffer);
		}
		else if((unsigned char)type == DATA_TYPE_STRING)
		{
			if(strlen((char *)buffer) > 100){
				// RETURN ERROR
			}

			strcpy(root->str, (char *)buffer);
			root->len = strlen(root->str);
		}

		return;
	}
	
	if((unsigned char)root->type == DATA_TYPE_STRING)
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
	else if((unsigned char)root->type == DATA_TYPE_INT)
	{
		if(*(int32_t *)(buffer) <  root->int_value)
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
		if((unsigned char)type == DATA_TYPE_INT)
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
		else if((unsigned char)type == DATA_TYPE_STRING)
		{
			if((unsigned char)type == DATA_TYPE_INT)
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

void TEST_show_all_nodes(struct graph_node * node)
{
	if(node == NULL)
		return;

	TEST_show_all_nodes(node->left);
	pr_info("Int value : %d\n", node->int_value);
	TEST_show_all_nodes(node->right);
}

void reset_traversal(struct graph_node * node)
{
	if(node == NULL)
		return;

	reset_traversal(node->left);
	node->travers = 0;
	reset_traversal(node->right);
}

// TODO: make it work for all the implementatios
struct graph_node * get_next_node(struct graph_node * node, int mode)
{
	if(mode == GRAPH_IN_ORDER)
	{
		if(node == NULL) return NULL;

		if(!(node->travers & TRAV_LEFT_DONE))
		{
			node->travers |= TRAV_LEFT_DONE;
			if(node->left) return get_next_node(node->left, mode);
			pr_info("calling itself (left) for %d\n", node->int_value);
			return get_next_node(node, mode);
		}
	
		else if(!(node->travers & TRAV_NODE_DONE))
		{
			node->travers |= TRAV_NODE_DONE;
			return node;
		}
	
		else if(!(node->travers & TRAV_RIGHT_DONE))
		{
			node->travers |= TRAV_RIGHT_DONE;
			if(node->right) return get_next_node(node->right, mode);
			pr_info("calling itself (right) for %d\n", node->int_value);
			return get_next_node(node, mode);
		}
	
		if(node->travers & (TRAV_RIGHT_DONE | TRAV_NODE_DONE | TRAV_LEFT_DONE))
		{
			return get_next_node(node->parent, mode);
		}
	
		return NULL;
	}

	else if(mode == GRAPH_POST_ORDER)
	{
		if(node == NULL) return NULL;

		if(!(node->travers & TRAV_LEFT_DONE))
		{
			node->travers |= TRAV_LEFT_DONE;
			if(node->left) return get_next_node(node->left, mode);
			pr_info("calling itself (left) for %d\n", node->int_value);
			return get_next_node(node, mode);
		}
	
		else if(!(node->travers & TRAV_RIGHT_DONE))
		{
			node->travers |= TRAV_RIGHT_DONE;
			if(node->right) return get_next_node(node->right, mode);
			pr_info("calling itself (right) for %d\n", node->int_value);
			return get_next_node(node, mode);
		}
		
		else if(!(node->travers & TRAV_NODE_DONE))
		{
			node->travers |= TRAV_NODE_DONE;
			return node;
		}
	
		if(node->travers & (TRAV_RIGHT_DONE | TRAV_NODE_DONE | TRAV_LEFT_DONE))
		{
			return get_next_node(node->parent, mode);
		}
	
		return NULL;
	}


	else if(mode == GRAPH_PRE_ORDER)
	{
		if(node == NULL) return NULL;
		
		if(!(node->travers & TRAV_NODE_DONE))
		{
			node->travers |= TRAV_NODE_DONE;
			return node;
		}

		else if(!(node->travers & TRAV_LEFT_DONE))
		{
			node->travers |= TRAV_LEFT_DONE;
			if(node->left) return get_next_node(node->left, mode);
			pr_info("calling itself (left) for %d\n", node->int_value);
			return get_next_node(node, mode);
		}
	
		else if(!(node->travers & TRAV_RIGHT_DONE))
		{
			node->travers |= TRAV_RIGHT_DONE;
			if(node->right) return get_next_node(node->right, mode);
			pr_info("calling itself (right) for %d\n", node->int_value);
			return get_next_node(node, mode);
		}
		
	
		if(node->travers & (TRAV_RIGHT_DONE | TRAV_NODE_DONE | TRAV_LEFT_DONE))
		{
			return get_next_node(node->parent, mode);
		}
	
		return NULL;
	}
	
	return NULL;
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

	//delete_graph(entry_ptr->graph);
	
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
	int32_t temp;
	struct graph_node * root;
	struct graph_node * node;

	pr_info("Creating Process : temp_proccess_entry\n");
	
	process_list_lock = (struct semaphore *) vmalloc(sizeof(struct semaphore));
	sema_init(process_list_lock, 1);

	process_list = (struct list_head *) vmalloc(sizeof(struct list_head));
	INIT_LIST_HEAD(process_list);	

	process_entry = proc_create("temp_process_entry", 0777, NULL, &process_operations);
	
	pr_info("Process Created\n");	

	temp = 10;
	root = create_root_node(&temp, DATA_TYPE_INT);

	pr_info("Root Created\n");
	
	// @TESTING: add_node function
	
	temp = 5;
	add_node(root, &temp, DATA_TYPE_INT);
	
	temp = 6;
	add_node(root, &temp, DATA_TYPE_INT);
	
	temp = 7;
	add_node(root, &temp, DATA_TYPE_INT);
	
	temp = 8;
	add_node(root, &temp, DATA_TYPE_INT);
	
	// @TESTING: displaying the tree
	
	TEST_show_all_nodes(root);


	// @TESTING: trversal mechanics

	pr_info("Testing traversal\n");

	node = root;

	reset_traversal(node);


	while(node != NULL){
		
		node = get_next_node(node, GRAPH_PRE_ORDER);
		if(node)
		{
			pr_info("getting value %d\n", node->int_value);
		}
	}
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
