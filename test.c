#include "avl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <limits.h>

#define arr_len(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct {
	long num;
	avl_node_t avl_node;
} outer_t;

int comparator(outer_t *node1, outer_t *node2) {
	if (node1->num == node2->num)
		return 0;
	if (node1->num <  node2->num)
		return -1;
	return 1;
}

outer_t *extractor(avl_node_t *node) {
	return avl_getitem(node, outer_t, avl_node);
}

int main() {
	srandom(time(NULL));

	avl_root_t root = avl_newroot(extractor, comparator);
	outer_t nodes[200000];

	long min, max;
	for (int j = 0; j < 100; ++j) {
		min = LONG_MAX;
		max = LONG_MIN;
		for (size_t i = 0; i < arr_len(nodes); ++i) {
			long rand = random();
			if (rand < min)
				min = rand;
			if (rand > max)
				max = rand;
			nodes[i].num = rand;
			avl_node_t *out = avl_insert(&nodes[i].avl_node, &root);
			assert((out == NULL || avl_compare(&root, out, &nodes[i].avl_node) == 0) && out != &nodes[i].avl_node);
			assert(min == extractor(avl_getmin(&root))->num);
			assert(max == extractor(avl_getmax(&root))->num);
		}
		for (size_t i = 0; i < arr_len(nodes); ++i) {
			avl_node_t *out = avl_find(&nodes[i].avl_node, &root);
			assert(nodes[i].num == extractor(out)->num);
			assert(out);
		}
		for (size_t i = 0; i < arr_len(nodes); ++i) {
			avl_node_t *out = avl_delete(&nodes[i].avl_node, &root);
			assert(out == NULL || out == &nodes[i].avl_node ||
			       nodes[i].num == extractor(out)->num);
		}
		printf("test %03d finished succesfully\n", j+1);
	}
	puts("all tests finished successfully");
}
