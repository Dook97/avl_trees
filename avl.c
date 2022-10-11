#include "avl.h"
#include <stddef.h>
#include <stdbool.h>

/* --- MACROS ------------------------------------------------- */

#define ABS(x)		(((x) >  0)  ? (x) : -(x))
#define MAX(x, y)	(((x) > (y)) ? (x) :  (y))

/* --- INTERNAL FUNCTIONS ------------------------------------- */

/* choose next node on the path to node with given key according to BST invariant */
static avl_node_t **choose_son(avl_key_t key, avl_node_t *node) {
	return (key < node->key) ? &node->left_son : &node->right_son;
}

/* returns true if node with given key was found otherwise false
 * out will point to father's pointer to node with given key if such exists
 * if it doesn't it will point to father's pointer to last node visited by the find operation */
static bool avl_find_getaddr(avl_key_t key, avl_root_t *root, avl_node_t ***out) {
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
 * arguments are pointers to fathers' pointers to the nodes */
static void replace_node(avl_node_t **replaced, avl_node_t **replacement) {
	(*replacement)->sign = (*replaced)->sign;

	if ((*replaced)->left_son != NULL)
		(*replaced)->left_son->father  = *replacement;
	if ((*replaced)->right_son != NULL)
		(*replaced)->right_son->father = *replacement;

	avl_node_t *temp = (*replacement)->right_son;
	if (temp != NULL)
		temp->father = (*replacement)->father;

	(*replacement)->father    = (*replaced)->father;
	(*replacement)->left_son  = (*replaced)->left_son;
	if ((*replaced)->right_son != *replacement)
		(*replacement)->right_son = (*replaced)->right_son;

	*replaced    = *replacement;
	*replacement = temp;
}

/* returns pointer to father's pointer to minimal node in right subtree of root
 * expects right son of root to be non-null */
static avl_node_t **get_min_node(avl_node_t *node) {
	avl_node_t **min = &node->right_son;
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
 * arguments are named according to the left part of the diagram */
static void rotate(avl_node_t **ynode, bool left_to_right) {
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

/* node points to father of deleted/inserted node
 * after a successful delete/insert traverses the path upward, updates signs
 * and carries out any necessary rotations */
static void balance(avl_node_t *node, avl_root_t *root, bool from_left, bool after_delete) {
	while (node != NULL) {
		bool newbool = after_delete ^ !from_left;
		node->sign += (newbool ? +1 : -1);
		if (ABS(node->sign) == after_delete)
			return;

		avl_node_t *father = node->father;
		bool new_left = (father != NULL && node == father->left_son);

		if (ABS(node->sign) == 2) {
			avl_node_t **son = newbool ? &node->right_son : &node->left_son;
			int prevsign = (*son)->sign;
			if (ABS(node->sign + (*son)->sign) == 1)
				rotate(son, newbool);
			rotate(get_fathers_ptr(node, root), !newbool);
			if (!after_delete || prevsign == 0)
				return;
		}

		from_left = new_left;
		node = father;
	}
}

/* returns number of non-NULL sons of node */
static int get_number_of_sons(avl_node_t *node) {
	return !!node->left_son + !!node->right_son;
}

/* replace a node by a newly inserted one */
static void replace_by_new(avl_node_t **replaced, avl_node_t *replacement) {
	replacement->sign = (*replaced)->sign;

	if ((*replaced)->left_son != NULL)
		(*replaced)->left_son->father = replacement;
	if ((*replaced)->right_son != NULL)
		(*replaced)->right_son->father = replacement;

	replacement->left_son = (*replaced)->left_son;
	replacement->right_son = (*replaced)->right_son;
	replacement->father = (*replaced)->father;

	*replaced = replacement;
}

/* --- PUBLIC FUNCTIONS --------------------------------------- */

/* returns pointer to node with given key or NULL if it wasn't found */
avl_node_t *avl_find(avl_key_t key, avl_root_t *root) {
	avl_node_t **out;
	avl_find_getaddr(key, root, &out);
	return *out;
}

/* if a node with given key already existed in the tree it is replaced by
 * new_node and the pointer to it is returned, otherwise the node is inserted
 * and NULL is returned */
avl_node_t *avl_insert(avl_node_t *new_node, avl_root_t *root) {
	avl_node_t **ptr2father, *father;
	bool found = avl_find_getaddr(new_node->key, root, &ptr2father);
	father = *ptr2father;

	if (found) {
		replace_by_new(ptr2father, new_node);
		return father;
	}

	new_node->father = father;
	new_node->left_son = new_node->right_son = NULL;
	new_node->sign = 0;

	*((root->root_node == NULL) ? &root->root_node
				    : choose_son(new_node->key, father)) = new_node;
	if (father != NULL)
		balance(father, root, new_node->key < father->key, 0);

	return NULL;
}

/* returns pointer to deleted node or NULL if it wasn't found */
avl_node_t *avl_delete(avl_key_t key, avl_root_t *root) {
	avl_node_t **son, *node, *balance_start;
	if (!avl_find_getaddr(key, root, &son))
		return NULL;

	node = *son;
	bool from_left;
	if (get_number_of_sons(node) < 2) {
		balance_start = node->father;
		from_left = (node->father != NULL && node->father->left_son == node);
		*son = (node->left_son != NULL) ? node->left_son : node->right_son;
		if (*son != NULL)
			(*son)->father = node->father;
	} else {
		avl_node_t **min = get_min_node(node);
		balance_start = ((*min)->father->key != key) ? (*min)->father : *min;
		from_left = (balance_start->left_son == *min);
		replace_node(son, min);
	}
	balance(balance_start, root, from_left, 1);

	return node;
}
