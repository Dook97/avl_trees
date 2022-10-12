#ifndef avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60
#define avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60

#include <stddef.h>
#include <stdbool.h>

/* --- TYPES -------------------------------------------------- */

typedef struct avl_node {
	struct avl_node *left_son, *right_son, *father;
	int sign; // right subtree depth - left subtree depth
} avl_node_t;

/* a comparator function intended for structs wrapping avl_node
 *
 * returns <0 if item1 < item2
 * returns  0 if item1 = item2
 * returns >0 if item1 > item2
 */
typedef int (*comparator_t)(void *item1, void *item2);

/* extract wrapper struct from avl_node */
typedef void *(*extractor_t)(avl_node_t *node);

typedef struct {
	avl_node_t *root_node;
	extractor_t extractor;
	comparator_t comparator;
} avl_root_t;

/* --- FUNCTIONS ---------------------------------------------- */

/* a shortcut to compare two nodes via the user provided extractor and comparator functions */
int avl_compare(avl_root_t *root, avl_node_t *node1, avl_node_t *node2);

/* initialize the root of the tree */
void avl_init_root(avl_root_t *root, extractor_t extractor, comparator_t comparator);

/* returns pointer to node with given key or NULL if it wasn't found */
avl_node_t *avl_find(avl_node_t *key_node, avl_root_t *root);

/* if a node with given key already existed in the tree it is replaced by
 * new_node and the pointer to it is returned, otherwise the node is inserted
 * and NULL is returned */
avl_node_t *avl_insert(avl_node_t *new_node, avl_root_t *root);

/* returns pointer to deleted node or NULL if it wasn't found */
avl_node_t *avl_delete(avl_node_t *key_node, avl_root_t *root);

/* --- MACROS ------------------------------------------------- */

/* compute offset of a member in a struct */
#define GET_MEMBER_OFFSET(wrapper_type, member_name) \
	((size_t)&((wrapper_type *)0)->member_name)

/* upcast from struct member to its wrapper struct */
#define GET_WRAPPER_STRUCT(ptr_to_member, wrapper_type, member_name) \
	({ \
		const __typeof__(((wrapper_type *)0)->member_name)* __mptr = (ptr_to_member); \
		(wrapper_type *)((void *)(__mptr) - GET_MEMBER_OFFSET(wrapper_type, member_name)); \
	})

/* upcast from struct member to its wrapper struct */
#define avl_getitem(ptr_to_avl_member, wrapper_type, avl_member_name) \
	GET_WRAPPER_STRUCT(ptr_to_avl_member, wrapper_type, avl_member_name)

#endif
