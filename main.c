#ifndef avl_guard_8a0aaa8e0dd0b46c5828e9450956b6576424bd98
#define avl_guard_8a0aaa8e0dd0b46c5828e9450956b6576424bd98

#include <stdint.h>
#include <stddef.h>

#include <assert.h>
#include <stdio.h>

typedef struct node {
	uint32_t key;
	struct node *left_son, *right_son, *father;
} avl_node_t;

/* return value indicates whether vertex with given key was found
 * ---
 * sets out to point to the father of the vertex which would've been the one
 * we're looking for if it existed or to the vertex itself if present
 */
int avl_find(int32_t key, avl_node_t *root, avl_node_t **out) {
	avl_node_t *current_node, *current_son;
	current_node = current_son = root;
	while (current_son != NULL && current_node->key != key) {
		current_node = current_son;
		current_son  = *(&current_node->left_son + !!(key > current_node->key));
	}
	*out = current_node;

	return current_node != NULL && key == current_node->key;
}

/* if there is a vertex with key equal to that of new_node, returns 1
 * otherwise inserts new_node and returns 0
 */
int avl_insert(avl_node_t *new_node, avl_node_t *root) {
	avl_node_t *father;
	if (avl_find(new_node->key, root, &father))
		return 1;

	new_node->father = father;
	*(&father->left_son + !!(new_node->key > father->key)) = new_node;

	return 0;
}

static void replace_node(avl_node_t *dest, avl_node_t *src) {
	dest->key = src->key;
}

/* if there is no vertex with given key, returns 1
 * otherwise deletes the appropriate node and returns 0
 */
int avl_delete(int32_t key, avl_node_t *root) {
	avl_node_t *node;
	if (!avl_find(key, root, &node))
		return 1;

	/* pointer to father's pointer to node */
	avl_node_t **ptr_to_son;
	if (node->father->left_son == NULL) {
		ptr_to_son = &(node->father->right_son);
	} else {
		ptr_to_son = (node->father->left_son->key == node->key) ?
			&(node->father->left_son) : &(node->father->right_son);
	}

	if (!!node->left_son + !!node->right_son < 2) {
		*ptr_to_son = (node->left_son != NULL) ? node->left_son : node->right_son;
	} else {
		avl_node_t *min = node->right_son;
		while (min->left_son != NULL)
			min = min->left_son;
		avl_delete(min->key, root);
		replace_node(node, min);
	}

	return 0;
}

void avl_show(avl_node_t *root) {
	if (root->left_son)
		avl_show(root->left_son);
	printf("%d\n", root->key);
	if (root->right_son)
		avl_show(root->right_son);
}

int main() {
	avl_node_t one, two, three, four, five, six;
	avl_node_t *ptr_to_last;
	one   = (avl_node_t){1};
	four  = (avl_node_t){4};
	three = (avl_node_t){3};
	two   = (avl_node_t){2};
	six   = (avl_node_t){6};
	five  = (avl_node_t){5};

	avl_insert(&four, &one);
	avl_insert(&three, &one);
	avl_insert(&two, &one);
	avl_insert(&six, &one);
	avl_insert(&five, &one);

	avl_show(&one);
	avl_delete(4, &one);
	printf("\n");
	avl_show(&one);
}

#endif
