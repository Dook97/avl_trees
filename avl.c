#include <stdint.h>
#include <stddef.h>
#include "avl.h"

/* --- MACROS ------------------------------------------------- */

#define ABS(x)		(((x) >  0)  ? (x) : -(x))
#define MAX(x, y)	(((x) > (y)) ? (x) :  (y))

/* --- INTERNAL FUNCTIONS ------------------------------------- */

/* choose next node on the path to node with given key according to BST invariant */
static avl_node_t **choose_son(avl_key_t key, avl_node_t *node) {
	return (key < node->key) ? &node->left_son : &node->right_son;
}

/* returns 1 if node with given key was found otherwise 0
 * out will point to father's pointer to node with given key if such exists
 * if it doesn't it will point to father's pointer to last node visited by the find operation
 */
static int avl_find_getaddr(avl_key_t key, avl_root_t *root, avl_node_t ***out) {
	avl_node_t **current_node, **current_son;
	current_node = current_son = &root->root_node;
	while (*current_son != NULL && (*current_node)->key != key) {
		current_node = current_son;
		current_son  = choose_son(key, *current_node);
	}
	*out = current_node;
	return *current_node != NULL && key == (*current_node)->key;
}

/* used exclusively inside avl_delete - DO NOT USE ELSEWHERE
 * handles changes of pointers between node with two sons and it's replacement
 * such deleted node is replaced with minimal node from it's right subtree
 * arguments are pointers to fathers' pointers to the nodes
 */
static void replace_node(avl_node_t **replaced, avl_node_t **replacement) {
	(*replacement)->sign = (*replaced)->sign;

	if ((*replaced)->left_son != NULL)
		(*replaced)->left_son->father = *replacement;
	if ((*replaced)->right_son != NULL)
		(*replaced)->right_son->father = *replacement;

	(*replacement)->father = (*replaced)->father;

	avl_node_t *temp = (*replacement)->right_son;

	(*replacement)->left_son  = (*replaced)->left_son;
	(*replacement)->right_son = (*replaced)->right_son;

	*replaced = *replacement;

	*replacement = temp;
}

/* returns number of non-null sons */
static int get_number_of_sons(avl_node_t *node) {
	return !!node->left_son + !!node->right_son;
}

/* returns pointer to father's pointer to minimal node in right subtree of root
 * expects right son of root to be non-null
 */
static avl_node_t **get_min_node(avl_node_t *root) {
	avl_node_t **min = &root->right_son;
	while ((*min)->left_son != NULL)
		min = &(*min)->left_son;
	return min;
}

/* edge rotation
 *     |           |
 *     y           x
 *    / \         / \
 *   x   C  <->  A   y
 *  / \             / \
 * A   B           B   C
 * it is presumed that x and y are non-null
 * x, y represent nodes while A, B, C represent (possibly empty) subtrees
 * arguments are named according to the left part of the diagram
 */
static void rotate(avl_node_t **ynode, int left_to_right) {
	avl_node_t **ptr_to_x = left_to_right ? &(*ynode)->left_son : &(*ynode)->right_son;
	avl_node_t *xnode = *ptr_to_x;
	avl_node_t **bnode = left_to_right ? &xnode->right_son : &xnode->left_son;

	/* update signs */
	int aheight, bheight;
	aheight = bheight = (left_to_right ? -(*ynode)->sign : (*ynode)->sign) - 1;
	if (xnode->sign < 0) {
		bheight = aheight + xnode->sign;
	} else {
		aheight = bheight - xnode->sign;
	}
	(*ynode)->sign = left_to_right ? -bheight : aheight;
	xnode->sign    = left_to_right ?  MAX(bheight, 0) - aheight + 1
				       : -MAX(aheight, 0) + bheight - 1;
	/* make b son of y */
	if (*bnode != NULL)
		(*bnode)->father = *ynode;
	*ptr_to_x = *bnode;

	/* move x to top */
	xnode->father = (*ynode)->father;
	(*ynode)->father = xnode;
	*bnode = *ynode;
	*ynode = xnode;
}

/* get pointer to father's pointer to node */
static avl_node_t **get_fathers_ptr(avl_node_t *node, avl_root_t *root) {
	if (node->father == NULL)
		return &root->root_node;
	return (node->father->left_son == node) ? &node->father->left_son : &node->father->right_son;
}

/* node is father of inserted node
 * after a successful insert traverses the path upward, updates signs and
 * carries out any necessary rotations
 */
static void balance_insert(avl_node_t *node, avl_root_t *root, int from_left) {
	while (node != NULL) {
		node->sign += (from_left ? -1 : +1);
		if (node->sign == 0)
			return;

		avl_node_t *father = node->father;
		int new_left = (father != NULL && node == father->left_son);

		if (ABS(node->sign) == 2) {
			avl_node_t **ptr_to_son = from_left ? &node->left_son : &node->right_son;
			if (ABS(node->sign + (*ptr_to_son)->sign) == 1)
				rotate(ptr_to_son, !from_left);
			rotate(get_fathers_ptr(node, root), from_left);

			return;
		}

		from_left = new_left;
		node = father;
	}
}

/* --- PUBLIC FUNCTIONS --------------------------------------- */

/* returns 1 if node with given key was found otherwise 0
 * out will point to node with given key if such exists (use out == NULL to discard)
 * if it doesn't it will point to last node visited by the find operation
 */
int avl_find(avl_key_t key, avl_root_t *root, avl_node_t **out) {
	avl_node_t **out_local;
	int ret = avl_find_getaddr(key, root, &out_local);
	if (out != NULL)
		*out = *out_local;
	return ret;
}

/* if there is a node with key equal to that of new_node returns 1
 * otherwise inserts new_node and returns 0
 */
int avl_insert(avl_node_t *new_node, avl_root_t *root) {
	avl_node_t *father;
	if (avl_find(new_node->key, root, &father))
		return 1;

	new_node->father = father;
	new_node->left_son = new_node->right_son = NULL;
	new_node->sign = 0;

	*((root->root_node == NULL) ? &root->root_node
				    : choose_son(new_node->key, father)) = new_node;
	if (father != NULL)
		balance_insert(father, root, new_node->key < father->key);

	return 0;
}

/* if there is no node with given key, returns 1
 * otherwise deletes the appropriate node and returns 0
 * pointer to deleted node is stored in the deleted variable (use NULL to discard)
 */
int avl_delete(avl_key_t key, avl_root_t *root, avl_node_t **deleted) {
	avl_node_t **ptr_to_son;
	if (!avl_find_getaddr(key, root, &ptr_to_son))
		return 1;

	avl_node_t *node = *ptr_to_son;
	if (get_number_of_sons(node) < 2) {
		*ptr_to_son = (node->left_son != NULL) ? node->left_son : node->right_son;
	} else {
		avl_node_t **min = get_min_node(node);
		replace_node(ptr_to_son, min);
	}

	if (deleted != NULL)
		*deleted = node;

	return 0;
}

/* --- DEBUG -------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void avl_enumerate(avl_node_t *root, int depth) {
	if (root == NULL) {
		printf("%*c-\n", depth, ' ');
		return;
	}
	printf("%*c%d\n", depth, ' ', root->key);
	avl_enumerate(root->left_son, depth + 1);
	avl_enumerate(root->right_son, depth + 1);
}

int main() {
	srandom(time(NULL));

	avl_root_t root = { .root_node = NULL };
	avl_node_t nodes[10];
	for (int i = 0; i < sizeof(nodes) / sizeof(avl_node_t); ++i) {
		nodes[i] = (avl_node_t){random() % 100};
		avl_insert(&nodes[i], &root);
	}
	avl_enumerate(root.root_node, 1);

	puts("");
	for (int i = 0; i < sizeof(nodes) / sizeof(avl_node_t); ++i)
		printf("key: %-2d    sign: %2d\n", nodes[i].key, nodes[i].sign);

	avl_delete(nodes[0].key, &root, NULL);
	/* avl_delete(nodes[1].key, &root, NULL); */
	/* avl_delete(nodes[2].key, &root, NULL); */
	/* avl_delete(nodes[3].key, &root, NULL); */
	/* avl_delete(nodes[4].key, &root, NULL); */

	avl_enumerate(root.root_node, 1);

	puts("");
	for (int i = 0; i < sizeof(nodes) / sizeof(avl_node_t); ++i)
		printf("key: %-2d    sign: %2d\n", nodes[i].key, nodes[i].sign);
}
