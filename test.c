#include "avl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

#define arr_len(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
	long num;
	avl_node_t avl_node;
} outer_t;

AVL_DEFINE_ROOT(outer_root_t, outer_t);

int comparator(outer_t *node1, outer_t *node2) {
	if (node1->num == node2->num)
		return 0;
	if (node1->num <  node2->num)
		return -1;
	return 1;
}

outer_t *extractor(avl_node_t *node) {
	return AVL_GETITEM(node, outer_t, avl_node);
}

int main() {
	outer_root_t root = AVL_NEWROOT(outer_root_t, extractor, comparator);
	outer_t nodes[100];

	for (size_t i = 0; i < arr_len(nodes); ++i) {
		nodes[i].num = i;
		avl_insert(&nodes[i].avl_node, &root);
	}

	outer_t test = { .num = -136 };
	outer_t test2 = { .num = 35 };

	avl_iterator_t iterator = avl_newiterator(&root, &test.avl_node, &test2.avl_node, false);
	for (outer_t *out; (out = avl_advance(&iterator, &root));)
		printf("%ld\n", out->num);
}
