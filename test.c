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
	// srandom(time(NULL));

	outer_root_t root = AVL_NEWROOT(outer_root_t, extractor, comparator);
	outer_t nodes[200000];

	// long min, max;
	for (int j = 0; j < 100; ++j) {
		// max = LONG_MIN;
		// min = LONG_MAX;

		for (size_t i = 0; i < arr_len(nodes); ++i) {
			// long rand = random();
			// if (rand < min)
			// 	min = rand;
			// if (rand > max)
			// 	max = rand;
			nodes[i].num = i;
			outer_t *out = avl_insert(&nodes[i].avl_node, &root);
			assert(out == NULL || (comparator(out, &nodes[i]) == 0 && out != &nodes[i]));
		}

		for (size_t i = 0; i < arr_len(nodes); ++i) {
			// printf("%ld %ld\n", i, avl_next(&nodes[i].avl_node, &root)->num);
			outer_t *out = avl_next(&nodes[i].avl_node, &root);
			outer_t *_out = avl_prev(&nodes[i].avl_node, &root);
			assert(out  == NULL || i + 1 ==  out->num);
			assert(_out == NULL || i - 1 == _out->num);
		}

		// assert(min == avl_min(&root)->num);
		// assert(max == avl_max(&root)->num);

		for (size_t i = 0; i < arr_len(nodes); ++i) {
			outer_t *out = avl_find(&nodes[i].avl_node, &root);
			assert(nodes[i].num == out->num);
			assert(out != NULL);
		}

		for (size_t i = 0; i < arr_len(nodes); ++i) {
			outer_t *out = avl_delete(&nodes[i].avl_node, &root);
			assert(out == NULL || out == &nodes[i] || nodes[i].num == out->num);
		}

		printf("test %03d finished succesfully\n", j+1);
	}
	puts("all tests finished successfully");
}
