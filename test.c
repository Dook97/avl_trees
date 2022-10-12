#include "avl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define arr_len(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct {
	int num;
	avl_node_t avl_node;
} outer_t;

int comparator(outer_t *node1, outer_t *node2) {
	return node1->num - node2->num;
}

outer_t *extractor(avl_node_t *node) {
	return avl_getitem(node, outer_t, avl_node);
}

int main() {
	srandom(time(NULL));

	avl_root_t root;
	avl_init_root(&root, (extractor_t)extractor, (comparator_t)comparator);

	outer_t nodes[200000];

	for (int j = 0; j < 100; ++j) {
		for (size_t i = 0; i < arr_len(nodes); ++i) {
			nodes[i] = (outer_t){ .num = random() };
			avl_node_t *out = avl_insert(&nodes[i].avl_node, &root);
			assert(out == NULL || avl_compare(&root, out, &nodes[i].avl_node) == 0 && out != &nodes[i].avl_node);
		}
		for (size_t i = 0; i < arr_len(nodes); ++i) {
			avl_node_t *out = avl_find(&nodes[i].avl_node, &root);
			assert(nodes[i].num == avl_getitem(out, outer_t, avl_node)->num);
			assert(out);
		}
		for (size_t i = 0; i < arr_len(nodes); ++i) {
			avl_node_t *out = avl_delete(&nodes[i].avl_node, &root);
			assert(out == NULL || out == &nodes[i].avl_node ||
			       nodes[i].num == avl_getitem(out, outer_t, avl_node)->num);
		}
		printf("test %03d finished succesfully\n", j+1);
	}

	puts("all tests finished successfully");
}
