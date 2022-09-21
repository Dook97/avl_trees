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

/* returns 1 if node with given key was found otherwise 0
 * out will point to node with given key if such exists (use out == NULL to discard)
 * if it doesn't it will point to last node visited by the find operation
 */
int avl_find(avl_key_t key, avl_root_t *root, avl_node_t **out);

/* if there is a node with key equal to that of new_node returns 1
 * otherwise inserts new_node and returns 0
 */
int avl_insert(avl_node_t *new_node, avl_root_t *root);

/* if there is no node with given key, returns 1
 * otherwise deletes the appropriate node and returns 0
 * pointer to deleted node is stored in the deleted variable (use NULL to discard)
 */
int avl_delete(avl_key_t key, avl_root_t *root, avl_node_t **deleted);

#endif
