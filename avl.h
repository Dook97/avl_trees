#ifndef avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60
#define avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60

#include <stdint.h>
#include <stddef.h>

/* --- TYPES -------------------------------------------------- */

typedef uint32_t avl_key_t;

typedef struct avl_node {
	avl_key_t key;
	struct avl_node *left_son, *right_son, *father;
	int sign; // right subtree depth - left subtree depth
} avl_node_t;

typedef struct {
	avl_node_t *root_node;
} avl_root_t;

/* --- FUNCTIONS ---------------------------------------------- */

/* returns pointer to node with given key or NULL if it wasn't found */
avl_node_t *avl_find(avl_key_t key, avl_root_t *root);

/* returns pointer to inserted node or NULL if a node with given key was already in the structure */
avl_node_t *avl_insert(avl_node_t *new_node, avl_root_t *root);

/* returns pointer to deleted node or NULL if it wasn't found */
avl_node_t *avl_delete(avl_key_t key, avl_root_t *root);

#endif
