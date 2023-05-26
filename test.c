#include "avl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

/* --- MACROS --------------------------------------- */

#define arr_len(arr) (sizeof(arr) / sizeof(arr[0]))
#define xstr(a) str(a)
#define str(a) #a
#define TEST_FAIL_IF_NOT(cond) \
	{ if (!(cond)) return "ERROR on line " xstr(__LINE__) " in " __FILE__; }

#define NODES_COUNT	500000
#define TEST_REPEAT	10

#define THUMBSUP	"\xf0\x9f\x91\x8d"
#define SADFACE		"\xf0\x9f\x98\xa5"

#define GREEN(str)	"\033[1;32m"str"\033[0m"
#define RED(str)	"\033[1;91m"str"\033[0m"

/* --- TYPEDEFS ------------------------------------- */

typedef struct {
	long num;
	avl_node_t avl_node;
} outer_t;

AVL_DEFINE_ROOT(outer_root_t, outer_t);

typedef char *(*test_func)(outer_root_t *, outer_t[]);

typedef struct {
	test_func test;
	char *msg;
	int repeat;
} testctx_t;

/* --- HELPER FUNCTIONS ------------------------------ */

int comparator(const void *node1, const void *node2) {
	long num1 = ((outer_t *)node1)->num, num2 = ((outer_t *)node2)->num;
	return (num1 == num2) ? 0
			      : (num1 < num2) ? -1 : +1;
}

void *safe_malloc(size_t size) {
	void *memory = malloc(size);
	if (memory == NULL) {
		fprintf(stderr, "couldn't allocate memory - exiting...\n");
		exit(1);
	}
	return memory;
}

void fill_random(outer_t nodes[]) {
	for (size_t i = 0; i < NODES_COUNT; ++i)
		nodes[i].num = random();
}

void fill_linear(outer_t nodes[]) {
	for (size_t i = 0; i < NODES_COUNT; ++i)
		nodes[i].num = i;
}

/* --- TEST FUNCTIONS ------------------------------- */

char *remove_all(outer_root_t *root, outer_t nodes[]) {
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		outer_t *deleted = avl_delete(root, &nodes[i]);
		TEST_FAIL_IF_NOT(deleted == NULL || comparator(deleted, &nodes[i]) == 0);
		TEST_FAIL_IF_NOT(!avl_contains(root, &nodes[i]));
	}
	return NULL;
}

char *insert_random(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	fill_random(nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		outer_t *replaced = avl_insert(root, &nodes[i]);
		TEST_FAIL_IF_NOT(replaced == NULL || comparator(replaced, &nodes[i]) == 0);
		TEST_FAIL_IF_NOT(avl_contains(root, &nodes[i]));
	}
	return NULL;
}

char *test_remove(outer_root_t *root, outer_t nodes[]) {
	insert_random(root, nodes);
	remove_all(root, nodes);
	return NULL;
}

char *insert_linear(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	fill_linear(nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		outer_t *replaced = avl_insert(root, &nodes[i]);
		TEST_FAIL_IF_NOT(replaced == NULL);
		TEST_FAIL_IF_NOT(avl_contains(root, &nodes[i]));
	}
	return NULL;
}

char *test_find(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	insert_random(root, nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		outer_t *found = avl_find(root, &nodes[i]);
		TEST_FAIL_IF_NOT(found->num == nodes[i].num);
	}
	return NULL;
}

char *test_min(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	insert_linear(root, nodes);
	TEST_FAIL_IF_NOT(comparator(avl_min(root), &nodes[0]) == 0);

	remove_all(root, nodes);
	insert_random(root, nodes);
	outer_t min = { .num = LONG_MAX };
	for (size_t i = 0; i < NODES_COUNT; ++i)
		if (comparator(&min, &nodes[i]) > 0)
			min = nodes[i];
	TEST_FAIL_IF_NOT(comparator(avl_min(root), &min) == 0);
	return NULL;
}

char *test_max(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	insert_linear(root, nodes);
	TEST_FAIL_IF_NOT(comparator(avl_max(root), &nodes[NODES_COUNT - 1]) == 0);

	remove_all(root, nodes);
	insert_random(root, nodes);
	outer_t max = { .num = LONG_MIN };
	for (size_t i = 0; i < NODES_COUNT; ++i)
		if (comparator(&max, &nodes[i]) < 0)
			max = nodes[i];
	TEST_FAIL_IF_NOT(comparator(avl_max(root), &max) == 0);
	return NULL;
}

char *test_next(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	insert_linear(root, nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		outer_t *next = avl_next(root, &nodes[i]);
		TEST_FAIL_IF_NOT((next == NULL && i == NODES_COUNT - 1) || next->num == (long)i + 1);
	}

	TEST_FAIL_IF_NOT(avl_next(root, &nodes[NODES_COUNT - 1]) == NULL);

	remove_all(root, nodes);
	insert_random(root, nodes);
	outer_t *nodes_copy = safe_malloc(NODES_COUNT * sizeof(outer_t));
	memcpy(nodes_copy, nodes, NODES_COUNT * sizeof(outer_t));
	qsort(nodes_copy, NODES_COUNT, sizeof(outer_t), comparator);
	outer_t *cur = avl_min(root);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		if (i < NODES_COUNT - 1 && comparator(&nodes_copy[i], &nodes_copy[i+1]) == 0)
			continue;
		TEST_FAIL_IF_NOT(comparator(&nodes_copy[i], cur) == 0);
		cur = avl_next(root, cur);
	}

	free(nodes_copy);
	return NULL;
}

char *test_prev(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	insert_linear(root, nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		outer_t *prev = avl_prev(root, &nodes[i]);
		TEST_FAIL_IF_NOT((prev == NULL && i == 0) || prev->num == (long)i - 1);
	}

	TEST_FAIL_IF_NOT(avl_prev(root, &nodes[0]) == NULL);

	remove_all(root, nodes);
	insert_random(root, nodes);
	outer_t *nodes_copy = safe_malloc(NODES_COUNT * sizeof(outer_t));
	memcpy(nodes_copy, nodes, NODES_COUNT * sizeof(outer_t));
	qsort(nodes_copy, NODES_COUNT, sizeof(outer_t), comparator);
	outer_t *cur = avl_max(root);
	for (size_t i = NODES_COUNT - 1; i > 0; --i) {
		if (i > 0 && comparator(&nodes_copy[i], &nodes_copy[i-1]) == 0)
			continue;
		TEST_FAIL_IF_NOT(comparator(&nodes_copy[i], cur) == 0);
		cur = avl_prev(root, cur);
	}

	free(nodes_copy);
	return NULL;
}

char *test_iterator(outer_root_t *root, outer_t nodes[]) {
	remove_all(root, nodes);
	insert_random(root, nodes);

	size_t offset = root->AVL_ROOT_EMBED.offset;

	avl_iterator_t iter = avl_get_iterator(root, &nodes[0], &nodes[1]);

	TEST_FAIL_IF_NOT(avl_peek(root, &iter) == NULL || comparator(avl_peek(root, &iter), &nodes[0]) == 0);
	TEST_FAIL_IF_NOT(avl_peek(root, &iter) == NULL || comparator(AVL_UPCAST(iter.end, offset), &nodes[1]) == 0);

	outer_t *prev = avl_advance(root, &iter);
	for (outer_t *cur; (cur = avl_advance(root, &iter));) {
		long end = ((outer_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF_NOT(cur->num <= end);
		TEST_FAIL_IF_NOT(comparator(prev, cur) < 0);
		prev = cur;
	}

	iter = avl_get_iterator(root, &nodes[0], &nodes[1], AVL_DESCENDING);
	prev = avl_advance(root, &iter);
	for (outer_t *cur; (cur = avl_advance(root, &iter));) {
		long end = ((outer_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF_NOT(cur->num >= end);
		TEST_FAIL_IF_NOT(comparator(prev, cur) > 0);
		prev = cur;
	}

	outer_t low = { .num = 1000 };
	outer_t hig = { .num = 9999 };
	iter = avl_get_iterator(root, &low, &hig);
	for (outer_t *cur; (cur = avl_advance(root, &iter)) != NULL;) {
		long end = ((outer_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF_NOT(cur->num <= end);
		TEST_FAIL_IF_NOT(comparator(cur, &low) >= 0);
		TEST_FAIL_IF_NOT(comparator(cur, &hig) <= 0);
	}

	iter = avl_get_iterator(root, &low, &hig, AVL_DESCENDING);
	prev = avl_advance(root, &iter);
	for (outer_t *cur; (cur = avl_advance(root, &iter));) {
		long end = ((outer_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF_NOT(cur->num >= end);
		TEST_FAIL_IF_NOT(comparator(prev, cur) > 0);
		prev = cur;
	}

	remove_all(root, nodes);
	insert_linear(root, nodes);

	iter = avl_get_iterator(root, &nodes[0], &nodes[NODES_COUNT - 1]);
	for (size_t i = 0; i < NODES_COUNT; ++i)
		TEST_FAIL_IF_NOT((long)i == avl_advance(root, &iter)->num);

	outer_t under = { .num = -100 };
	iter = avl_get_iterator(root, &under, &nodes[0]);
	TEST_FAIL_IF_NOT(avl_peek(root, &iter) == avl_min(root));

	iter = avl_get_iterator(root, &nodes[NODES_COUNT - 1], &nodes[0]);
	TEST_FAIL_IF_NOT(avl_advance(root, &iter) == NULL);

	return NULL;
}

/* --- TEST INFRASTRUCUTRE -------------------------- */

int run_test(testctx_t *ctx, outer_root_t *root, outer_t nodes[]) {
	int err_counter = 0, lasterr = 0;
	char *strerr = NULL;
	for (int i = 1; i <= ctx->repeat; ++i) {
		printf("%c%-25s%2d/%d", lasterr ? '\n' : '\r', ctx->msg, i, ctx->repeat);
		fflush(stdout);
		lasterr = 0;
		if ((strerr = ctx->test(root, nodes)) != NULL) {
			printf(RED("\t%s"), strerr);
			++err_counter;
			lasterr = 1;
		}
	}

	if (err_counter) {
		printf("%c%-32s%s\n", lasterr ? '\n' : '\r', ctx->msg, RED("FAILED"));
		fflush(stdout);
	} else {
		puts(GREEN("\tOK"));
	}

	return err_counter;
}

int main(void) {
	srandom(time(NULL));
	outer_root_t root = AVL_NEW(outer_root_t, avl_node, comparator);
	outer_t *nodes = safe_malloc(NODES_COUNT * sizeof(outer_t));

	testctx_t ctxs[] = {
		{ .test = insert_random, .msg = "random_insert", .repeat = TEST_REPEAT },
		{ .test = insert_linear, .msg = "linear_insert", .repeat = TEST_REPEAT },
		{ .test = test_remove,   .msg = "remove",        .repeat = TEST_REPEAT },
		{ .test = test_find,     .msg = "find",          .repeat = TEST_REPEAT },
		{ .test = test_min,      .msg = "min",           .repeat = TEST_REPEAT },
		{ .test = test_max,      .msg = "max",           .repeat = TEST_REPEAT },
		{ .test = test_next,     .msg = "next",          .repeat = TEST_REPEAT },
		{ .test = test_prev,     .msg = "prev",          .repeat = TEST_REPEAT },
		{ .test = test_iterator, .msg = "iterator",      .repeat = TEST_REPEAT },
	};

	int err_counter = 0;
	for (int i = 0; i < arr_len(ctxs); ++i)
		err_counter += run_test(&ctxs[i], &root, nodes);

	free(nodes);
	if (err_counter == 0) {
		printf("\nAll tests passed successfully! " THUMBSUP "\n");
		return 0;
	} else {
		printf("\n%d tests failed " SADFACE "\n", err_counter);
		return 1;
	}
}
