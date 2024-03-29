// TODO 
//
//	Get the get_graph_info working and add the read/write/ioctl interfaces
//	for the program
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

#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "graph_module.h"

MODULE_LICENSE("GPL");

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
	node->type = type;

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
		if(strcmp((char *)buffer, root->str) < 0)
		{	
			add_node(get_left_node(root), buffer, type);
		}
		else
		{
			add_node(get_right_node(root), buffer, type);
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

	return NULL;
}

// The following is the group of functions used for traversal 
// through the graph
// 	- nitesh

// TODO:
// 	test the traversal order using inmodule function calls
// 	- nitesh

void TEST_show_all_nodes(struct graph_node * node, int space )
{
	int i= 0 ;
	if(node == NULL)
		return;

	TEST_show_all_nodes(node->left, space + 1);
	printk(KERN_INFO "\t");
	for(i =0 ; i < space;  i++) printk(KERN_CONT " ");
	printk(KERN_CONT "%d\n", node->int_value);
	TEST_show_all_nodes(node->right, space + 1);
}

void reset_traversal(struct graph_node * node)
{
	if(node == NULL)
		return;

	reset_traversal(node->left);
	node->travers = 0;
	reset_traversal(node->right);
}

struct graph_node * get_next_node(struct graph_node * node, int mode)
{
	if(mode == GRAPH_IN_ORDER)
	{
		if(node == NULL) return NULL;

		if(!(node->travers & TRAV_LEFT_DONE))
		{
			node->travers |= TRAV_LEFT_DONE;
			if(node->left) return get_next_node(node->left, mode);
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
			return get_next_node(node, mode);
		}
	
		else if(!(node->travers & TRAV_RIGHT_DONE))
		{
			node->travers |= TRAV_RIGHT_DONE;
			if(node->right) return get_next_node(node->right, mode);
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
			return get_next_node(node, mode);
		}
	
		else if(!(node->travers & TRAV_RIGHT_DONE))
		{
			node->travers |= TRAV_RIGHT_DONE;
			if(node->right) return get_next_node(node->right, mode);
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
// Function for deleteing the graph
void delete_graph(struct graph_node * root)
{
	if(root == NULL) return;

	delete_graph(root->left);
	delete_graph(root->right);
	
	root->left = NULL;
	root->right = NULL;	

	vfree(root);
}

// Functions used for getting information regarding the BST in the LKM
// 
void traverse_with_update(struct graph_node * node, int * maxdepth, int * mindepth, int * deg1, int * deg2, int * deg3, int depth)
{
	int nullcount = 0;
	if(node == NULL) return;
	traverse_with_update(node->left, maxdepth, mindepth, deg1, deg2, deg3, depth+1);

	if(node->parent != NULL){
	       	nullcount++;
	}
	if(node->left != NULL) { 
                nullcount++;
        }
	if(node->right != NULL) { 
                nullcount++;
        }

	if(nullcount == 1) 	*deg1 += 1;
	else if(nullcount == 2) *deg2 += 1;
	else if(nullcount == 3) *deg3 += 1;

	if(node->left == NULL && node->right == NULL)
	{
		pr_info("reached leaf node with value %d\n", node->int_value);
		if(depth > *maxdepth)
		{
			*maxdepth = depth;
		}
		
		if(depth < *mindepth)
		{
			*mindepth = depth;
		}
	}


	traverse_with_update(node->right, maxdepth, mindepth, deg1, deg2, deg3, depth+1);
}


struct obj_info * get_graph_info(struct graph_node * root)
{
	struct obj_info * infoptr = NULL;

	int maxdepth = 0;
	int mindepth = INT_MAX;

	int deg1 = 0;
	int deg2 = 0;
	int deg3 = 0;

	traverse_with_update(root, &maxdepth, &mindepth, &deg1, &deg2, &deg3, 0);

	infoptr = (struct obj_info *) vmalloc(sizeof(struct obj_info));
//	pr_info("%d %d %d %d %d\n",maxdepth, mindepth , deg1, deg2, deg3);
	infoptr->maxdepth = maxdepth;
	infoptr->mindepth = mindepth;

	infoptr->deg1cnt = deg1;
	infoptr->deg2cnt = deg2;
	infoptr->deg3cnt = deg3;

	return infoptr;
}


//// ENDING



struct process_entry{
	
	int pid;
	
	int read_count;

	char type;
	int order;

	int state;
	
	struct graph_node * graph;
	struct graph_node * current_node;
	
	struct list_head node;
};

static void init_process_entry(struct process_entry * entry_ptr){
	if(entry_ptr == NULL)
		return;

	entry_ptr->graph 		= NULL;	
	entry_ptr->order		= GRAPH_IN_ORDER;
	entry_ptr->state		= 0;
	
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
	struct process_entry * entry_ptr = NULL;
	
	entry_ptr = get_process_entry(current->pid);

	if(entry_ptr == NULL)
	{
		return -EBADF;
	}
	
	// reading from process which has not set the type	
	else if((unsigned char)entry_ptr->type == DATA_TYPE_NONE)
	{
		return -EACCES;
	}

	
	if(entry_ptr->state != PROCESS_READ_STATE)
	{
		reset_traversal(entry_ptr->graph);
		entry_ptr->current_node = entry_ptr->graph;
		entry_ptr->state = PROCESS_READ_STATE;


		//TEST_show_all_nodes(entry_ptr->graph, 0);
	}


	entry_ptr->current_node = get_next_node(entry_ptr->current_node, entry_ptr->order);
	
	pr_info("%p is the current node \n", entry_ptr->current_node);

	if(entry_ptr->current_node == NULL) return 0;

	if((unsigned char)entry_ptr->type == DATA_TYPE_STRING)
	{
		memcpy(buffer, entry_ptr->current_node->str, entry_ptr->current_node->len);
		return entry_ptr->current_node->len;
	}
	else if((unsigned char)entry_ptr->type == DATA_TYPE_INT)
	{
		memcpy(buffer, &entry_ptr->current_node->int_value, sizeof(int32_t));
		pr_info("buffer read - %d\n", entry_ptr->current_node->int_value);
		return sizeof(int32_t);
	}

	return 0; 
}
/*
struct process_entry{
	
	int pid;
	
	int read_count;

	int type;
	
	struct graph_node * graph;
	
	struct list_head node;
};
*/

static long process_ioctl_handler(struct file * fptr, 
				  unsigned int flag, 
				  unsigned long arg)
{
	struct process_entry * entry_ptr = NULL;
	char chdata= 0;
	int ret = 0;
	struct obj_info * info = NULL;
	struct search_obj objptr ;
	struct graph_node * node = NULL;

	entry_ptr = get_process_entry(current->pid);
	if(entry_ptr == NULL)
	{
		return -EBADF;
	}

	entry_ptr->state = PROCESS_IOCTL_STATE;
	
	switch(flag)
	{
	case PB2_SET_TYPE:
	{
		pr_info("process %d -> ioctl setting type", entry_ptr->pid);
		ret = get_user(chdata, (char *)arg);
		if(ret) return -EINVAL;
	

		pr_info("value revieved %p, %d\n", (char *) arg, chdata);

		if((unsigned char)chdata!= DATA_TYPE_STRING && (unsigned char)chdata != DATA_TYPE_INT) return -EINVAL;

		entry_ptr->type = chdata;

		pr_info("assigning %d, %d\n", entry_ptr->type , chdata);

		return 0;
	}
	break;
	case PB2_SET_ORDER:
	{
		pr_info("process %d -> ioctl setting order", entry_ptr->pid);
		if(entry_ptr->type == DATA_TYPE_NONE) return -EACCES;

		ret = get_user(chdata, (char *)arg);
		if(ret) return -EINVAL;
		
		if((unsigned char)chdata != GRAPH_IN_ORDER && (unsigned char)chdata != GRAPH_POST_ORDER && (unsigned char)chdata != GRAPH_PRE_ORDER) return -EINVAL;
		
		entry_ptr->order = chdata;
		
		reset_traversal(entry_ptr->graph);

		return 0;
	}
	break;
	case PB2_GET_INFO:
	{
		// IMPLEMENT: @RahulKrantiKiran please implement 
		pr_info("process %d -> ioctl getting info",entry_ptr->pid );
		
		info = get_graph_info(entry_ptr->graph);
		
		pr_info("%d %d %d %d %d\n",info->deg1cnt, info->deg2cnt, info->deg3cnt, info->mindepth, info->maxdepth);
		
		ret = copy_to_user( (struct obj_info*)arg, info, sizeof(struct obj_info));
		
		if(!ret)
		{
			return -EINVAL;
		}	
		
		pr_info("The copy to user value is %d\n",ret);
		
		if(ret == 0)
			return 0;
		else
			return -EINVAL;
	}
	break;
	case PB2_GET_OBJ:
	{
		// IMPLEMENT: @RahulKrantiKiran please implement 
		pr_info("process %d -> ioctl search object", entry_ptr->pid);
		
		ret = copy_from_user(&objptr, (struct obj_info *) arg,  sizeof(struct search_obj));
		
		pr_info("Searching %s Sixze is %d and ret is %d\n",objptr.str, (int)sizeof(struct search_obj), ret);
		if(ret)
		{
			pr_info("Returning EINVAL\n");
			return -EINVAL;
		}
		
		if((unsigned char) objptr.objtype == DATA_TYPE_INT) node = search_graph(entry_ptr->graph, &objptr.int_obj);
		else node = search_graph(entry_ptr->graph, objptr.str);
		
		pr_info("The valus is %s\n",node->str);
		if(node == NULL) 
		{
			pr_info("Not found\n");
			objptr.found = 1;
			objptr.len = -1;
			ret = copy_to_user((struct obj_info *) arg, &objptr, sizeof(struct obj_info ));
			return 0;
		}
		
		objptr.found = 0;

		if(entry_ptr->type == DATA_TYPE_STRING)
		{
			objptr.len = strlen(node->str);
		}

		ret = copy_to_user((struct obj_info *) arg, &objptr, sizeof(struct obj_info ));
		
		if(ret)
		{
			pr_info("get EINVAL while returning\n");

			return -EINVAL;
		}
					
		return 0;
	}
	break;
	default:
	{
		return -EINVAL;
	}
	}
}

static ssize_t process_write_handler(struct file * fptr, 
				     const char __user * buffer, 
				     size_t size, 
				     loff_t * ppos)
{
	struct process_entry * entry_ptr = NULL;

	pr_info("write_handler being\n");

	entry_ptr = get_process_entry(current->pid);

	if(entry_ptr == NULL)
	{
		return -EBADF;
	}
	
	else if(entry_ptr->type == DATA_TYPE_NONE)
	{
		pr_info("data type is none\n");
		
		return -EACCES;
	}	
	
	entry_ptr->state = PROCESS_WRITE_STATE;

	if(entry_ptr->graph == NULL)
	{
		entry_ptr->graph = create_root_node((void *)buffer, entry_ptr->type);
		
	
	}
	else
	{
		add_node(entry_ptr->graph, (void *)buffer, entry_ptr->type);
		pr_info("recieving information %d\n", *(int32_t *) buffer);

	}

	if(entry_ptr->type == DATA_TYPE_STRING) return strlen(buffer);
	if(entry_ptr->type == DATA_TYPE_INT) return sizeof(int32_t);

	return 0;
} 

static struct file_operations process_operations = 
{
	.owner 	= THIS_MODULE,
	.open	= process_open_handler,
	.read	= process_read_handler,
	.unlocked_ioctl	= process_ioctl_handler,
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
	
	pr_info("Creating Process : partb_1_16CS30023_16CS10018\n");
	
	process_list_lock = (struct semaphore *) vmalloc(sizeof(struct semaphore));
	sema_init(process_list_lock, 1);

	process_list = (struct list_head *) vmalloc(sizeof(struct list_head));
	INIT_LIST_HEAD(process_list);	

	process_entry = proc_create("partb_2_16CS30023_16CS10018", 0777, NULL, &process_operations);
	
	pr_info("Process Created\n");	
	return 0;
}


static __exit void exit_module_assign(void)
{
	pr_info("Removing Process : partb_1_16CS30023_16CS10018\n");

	proc_remove(process_entry);

	clean_process_list();

	vfree(process_list);

	pr_info("Process Removed\n");
	return;
}


module_init(init_module_assign);
module_exit(exit_module_assign);
