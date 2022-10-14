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

int comparator(void *node1, void *node2) {
	if (((outer_t *)node1)->num == ((outer_t *)node2)->num)
		return 0;
	if (((outer_t *)node1)->num <  ((outer_t *)node2)->num)
		return -1;
	return 1;
}

void *extractor(avl_node_t *node) {
	return AVL_UPCAST(node, outer_t, avl_node);
}

int main() {
	outer_root_t root = AVL_NEW(outer_root_t, extractor, comparator);

	outer_t node1 = { .num = 1 };
	outer_t node2 = { .num = 2 };
	outer_t node3 = { .num = 5 };
	outer_t nodes[3] = {node1,node2,node3};

	nodes[0] = node1;
	nodes[1] = node2;
	nodes[2] = node3;
	for (size_t i = 0; i < arr_len(nodes); ++i) {
		avl_insert(&root, &nodes[i].avl_node);
	}

	outer_t test = { .num = 4 };

	avl_iterator_t iterator = avl_get_iterator(&root, NULL, &test.avl_node, false);
	for (outer_t *out; (out = avl_advance(&root, &iterator)); )
		printf("%ld\n", out->num);

	assert(avl_contains(&root, &node1.avl_node));
	assert(!avl_contains(&root, &test.avl_node));
}
