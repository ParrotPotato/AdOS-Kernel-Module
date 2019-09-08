/*
 * Name and Roll Nos :
 *      Nitesh Meena (16CS30023)
 *      G Rahul Kranti Kiran (16CS10018)
 *
 * Kernel Version : 
 *      5.2.9 
 */

#ifndef GRAPH_MODULE_HEADER
#define GRAPH_MODULE_HEADER


#define PB2_SET_TYPE 	 _IOW(0x10, 0x31, int32_t*)
#define PB2_SET_ORDER 	 _IOW(0x10, 0x32, int32_t*)
#define PB2_GET_INFO 	 _IOW(0x10, 0x33, int32_t*)
#define PB2_GET_OBJ	 _IOW(0x10, 0x34, int32_t*)

#define DATA_TYPE_NONE	 0x00
#define DATA_TYPE_INT	 0xff
#define DATA_TYPE_STRING 0xf0

#define GRAPH_IN_ORDER	 0x00
#define GRAPH_POST_ORDER 0x01
#define GRAPH_PRE_ORDER	 0x02

#define TRAV_LEFT_DONE 	 0x02
#define TRAV_RIGHT_DONE  0x04
#define TRAV_NODE_DONE	 0x08

#define PROCESS_WRITE_STATE 0x01
#define PROCESS_IOCTL_STATE 0x02
#define PROCESS_READ_STATE  0x03

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

#endif
