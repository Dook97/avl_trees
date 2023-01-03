#include "avl.h"

/* --- MACROS ------------------------------------------------- */

#define ABS(x) ({ __auto_type __temp_x = (x); __temp_x < 0 ? -__temp_x : __temp_x; })
#define MAX(x, y) \
	({ \
		 __auto_type __temp_x = (x); \
		 __auto_type __temp_y = (y); \
		 __temp_x > __temp_y ? __temp_x : __temp_y; \
	})

/* --- INTERNAL FUNCTIONS ------------------------------------- */

/* a shortcut to compare two nodes via the user provided comparator function
 *
 * returns <0 if node1 < node2
 * returns  0 if node1 = node2
 * returns >0 if node1 > node2
 */
static int compare_nodes(avl_root_t *root, avl_node_t *node1, avl_node_t *node2) {
	return (*root->comparator)(AVL_UPCAST(node1, root->offset),
				   AVL_UPCAST(node2, root->offset));
}

/* choose next node on the path to node with given key according to BST invariant */
static avl_node_t **choose_son(avl_node_t *key_node, avl_node_t *node, avl_root_t *root) {
	return (compare_nodes(root, key_node, node) < 0) ? &node->sons[left] : &node->sons[right];
}

/* returns true if node with given key was found otherwise false
 * out will point to father's pointer to node with given key if such exists
 * if it doesn't it will point to father's pointer to last node visited by the find operation */
static bool avl_find_getaddr(avl_node_t *key_node, avl_root_t *root, avl_node_t ***out) {
	avl_node_t **current_node, **current_son;
	current_node = current_son = &root->root_node;
	while (*current_son != NULL && compare_nodes(root, *current_node, key_node) != 0) {
		current_node = current_son;
		current_son  = choose_son(key_node, *current_node, root);
	}
	*out = current_node;
	return *current_node != NULL && compare_nodes(root, *current_node, key_node) == 0;
}

/* used exclusively inside avl_delete - DO NOT USE ELSEWHERE
 * handles changes of pointers between node with two sons and it's replacement
 * such deleted node is replaced with minimal node from it's right subtree
 * arguments are pointers to fathers' pointers to the nodes */
static void replace_node(avl_node_t **replaced, avl_node_t **replacement) {
	(*replacement)->sign = (*replaced)->sign;

	if ((*replaced)->sons[left] != NULL)
		(*replaced)->sons[left]->father  = *replacement;
	if ((*replaced)->sons[right] != NULL)
		(*replaced)->sons[right]->father = *replacement;

	avl_node_t *temp = (*replacement)->sons[right];
	if (temp != NULL)
		temp->father = (*replacement)->father;

	(*replacement)->father = (*replaced)->father;
	(*replacement)->sons[left] = (*replaced)->sons[left];
	if ((*replaced)->sons[right] != *replacement)
		(*replacement)->sons[right] = (*replaced)->sons[right];

	*replaced    = *replacement;
	*replacement = temp;
}

/* returns pointer to fathers pointer to minimum of the right subtree or maximum of the left subtree */
static avl_node_t **minmax_of_subtree(avl_node_t *node, bool max) {
	avl_node_t **min = &node->sons[max];
	while ((*min)->sons[!max] != NULL)
		min = &(*min)->sons[!max];
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
	avl_node_t **ptr_to_x = &(*ynode)->sons[!left_to_right];
	avl_node_t *xnode = *ptr_to_x;
	avl_node_t **bnode = &xnode->sons[left_to_right];

	/* update signs */
	int aheight, bheight;
	aheight = bheight = (left_to_right ? -(*ynode)->sign : (*ynode)->sign) - 1;
	if (xnode->sign < 0) {
		bheight += xnode->sign;
	} else {
		aheight -= xnode->sign;
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
	return (node->father->sons[left] == node) ? &node->father->sons[left] : &node->father->sons[right];
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
		bool new_left = (father != NULL && node == father->sons[left]);

		if (ABS(node->sign) == 2) {
			avl_node_t **son = &node->sons[newbool];
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
	return !!node->sons[left] + !!node->sons[right];
}

/* replace a node by a newly inserted one */
static void replace_by_new(avl_node_t **replaced, avl_node_t *replacement) {
	replacement->sign = (*replaced)->sign;

	if ((*replaced)->sons[left] != NULL)
		(*replaced)->sons[left]->father = replacement;
	if ((*replaced)->sons[right] != NULL)
		(*replaced)->sons[right]->father = replacement;

	replacement->sons[left]  = (*replaced)->sons[left];
	replacement->sons[right] = (*replaced)->sons[right];
	replacement->father	 = (*replaced)->father;

	*replaced = replacement;
}

/* initialize newly inserted node */
static void init_node(avl_node_t *node, avl_node_t *father) {
	node->father = father;
	node->sons[left] = node->sons[right] = NULL;
	node->sign = 0;
}

/* returns closest lower/higher node according to the ordering defined by the comparator function
 * if key_node itself is in the structure it is returned */
static avl_node_t *get_closest_node(avl_root_t *root, avl_node_t *key_node, bool higher) {
	avl_node_t *out = NULL;
	avl_node_t *temp = root->root_node;
	int comparison;
	while (temp != NULL) {
		comparison = compare_nodes(root, key_node, temp);
		if (comparison == 0)
			return temp;
		if ((!higher && comparison < 0) || (higher && comparison > 0)) {
			temp = temp->sons[higher];
			continue;
		}
		out = temp;
		temp = temp->sons[!higher];
	}
	return out;
}

/* get previous or next node according to the ordering specified by the comparator function */
static avl_node_t *prevnext(avl_node_t *node, bool next) {
	if (node->sons[next] != NULL) {
		node = node->sons[next];
		while (node->sons[!next])
			node = node->sons[!next];
		return node;
	}
	while (node->father != NULL && node == node->father->sons[next])
		node = node->father;
	return node->father;
}

/* --- PUBLIC FUNCTIONS --------------------------------------- */

/* returns pointer to node with given key or NULL if it wasn't found */
avl_node_t *avl_find_impl(avl_node_t *key_node, avl_root_t *root) {
	avl_node_t **out;
	return avl_find_getaddr(key_node, root, &out) ? *out : NULL;
}

/* if a node with given key already existed in the tree it is replaced by
 * new_node and the pointer to it is returned, otherwise the node is inserted
 * and NULL is returned */
avl_node_t *avl_insert_impl(avl_node_t *new_node, avl_root_t *root) {
	avl_node_t **ptr2father, *father;
	bool found = avl_find_getaddr(new_node, root, &ptr2father);
	father = *ptr2father;

	if (found) {
		replace_by_new(ptr2father, new_node);
		return father;
	}

	init_node(new_node, father);
	*((root->root_node == NULL) ? &root->root_node
				    : choose_son(new_node, father, root)) = new_node;

	if (father != NULL)
		balance(father, root, compare_nodes(root, new_node, father) < 0, false);

	return NULL;
}

/* returns pointer to deleted node or NULL if it wasn't found */
avl_node_t *avl_delete_impl(avl_node_t *key_node, avl_root_t *root) {
	avl_node_t **son, *node, *balance_start;
	if (!avl_find_getaddr(key_node, root, &son))
		return NULL;

	node = *son;
	bool from_left;
	if (get_number_of_sons(node) < 2) {
		balance_start = node->father;
		from_left = (node->father != NULL && node->father->sons[left] == node);
		*son = (node->sons[left] != NULL) ? node->sons[left] : node->sons[right];
		if (*son != NULL)
			(*son)->father = node->father;
	} else {
		avl_node_t **min = minmax_of_subtree(node, right);
		balance_start = (compare_nodes(root, (*min)->father, key_node) != 0) ? (*min)->father : *min;
		from_left = (balance_start->sons[left] == *min);
		replace_node(son, min);
	}
	balance(balance_start, root, from_left, true);

	return node;
}

/* get minimal or maximal node according to the ordering specified by the comparator function */
avl_node_t *avl_minmax_impl(avl_root_t *root, bool max) {
	avl_node_t uber_root = { .sons = {root->root_node, root->root_node} };
	return *minmax_of_subtree(&uber_root, !max);
}

/* returns closest lower/higher node according to the ordering defined by the comparator function
 * unlike get_closest_node this func doesnt return key_node when it is present in the structure */
avl_node_t *avl_prevnext_impl(avl_root_t *root, avl_node_t *key_node, bool next) {
	avl_node_t *out = get_closest_node(root, key_node, next);
	if (out != NULL && compare_nodes(root, key_node, out) == 0)
		out = prevnext(out, next);
	return out;
}

/* get new iterator */
avl_iterator_t avl_get_iterator_impl(avl_root_t *root, avl_node_t *lower_bound, avl_node_t *upper_bound, bool low_to_high) {
	avl_iterator_t out = {
		.cur = NULL,
		.root = root,
		.low_to_high = low_to_high
	};

	if (root->root_node == NULL)
		return out;

	avl_node_t *min = avl_minmax_impl(root, false);
	avl_node_t *max = avl_minmax_impl(root, true);

	avl_node_t *lower = (lower_bound == NULL) ? min : get_closest_node(root, lower_bound, true);
	avl_node_t *upper = (upper_bound == NULL) ? max : get_closest_node(root, upper_bound, false);

	/* if an invalid range is specified invalidate the iterator */
	if (lower == NULL || upper == NULL
		|| (upper_bound != NULL && compare_nodes(root, lower, upper_bound) > 0)
		|| (lower_bound != NULL && compare_nodes(root, upper, lower_bound) < 0)
		|| (lower_bound != NULL && compare_nodes(root, lower_bound, max)   > 0)
		|| (upper_bound != NULL && compare_nodes(root, upper_bound, min)   < 0)) {
		return out;
	}

	out.cur = low_to_high ? lower : upper;
	out.end = low_to_high ? upper : lower;
	return out;
}

/* get next node from iterator */
avl_node_t *avl_advance_impl(avl_iterator_t *iterator) {
	if (iterator->cur == NULL)
		return NULL;

	if (compare_nodes(iterator->root, iterator->cur, iterator->end) == 0) {
		iterator->cur = NULL;
		return iterator->end;
	}

	avl_node_t *out = iterator->cur;
	iterator->cur = prevnext(iterator->cur, iterator->low_to_high);
	return out;
}

/* get next node from iterator without changing its state */
avl_node_t *avl_peek_impl(avl_iterator_t *iterator) {
	return iterator->cur;
}
