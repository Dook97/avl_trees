#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>

#include "avl.h"

/* --- MACROS --------------------------------------- */

#define arr_len(arr) (sizeof(arr) / sizeof(arr[0]))
#define xstr(a) str(a)
#define str(a) #a
#define TEST_FAIL_IF(cond) \
	do { if ((cond)) return "ERROR on line " xstr(__LINE__) " in " __FILE__; } while (0)

#define NODES_COUNT	500000
#define TEST_REPEAT	10

#define THUMBSUP	"\xf0\x9f\x91\x8d"
#define SADFACE		"\xf0\x9f\x98\xa5"

#define GREEN(str)	"\033[1;32m" str "\033[0m"
#define RED(str)	"\033[1;91m" str "\033[0m"

/* --- TYPEDEFS ------------------------------------- */

typedef struct {
	long num;
	avl_node_t dict_data;
} dict_item_t;

AVL_DEFINE_ROOT(dict_t, dict_item_t);

typedef char *(*test_func)(dict_t *, dict_item_t[]);

typedef struct {
	test_func test;
	char *msg;
	int repeat;
} testctx_t;

/* --- HELPER FUNCTIONS ------------------------------ */

int comparator(const void *node1, const void *node2) {
	long num1 = ((dict_item_t *)node1)->num, num2 = ((dict_item_t *)node2)->num;
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

void fill_random(dict_item_t nodes[]) {
	for (size_t i = 0; i < NODES_COUNT; ++i)
		nodes[i].num = random();
}

void fill_linear(dict_item_t nodes[]) {
	for (size_t i = 0; i < NODES_COUNT; ++i)
		nodes[i].num = i;
}

/* --- TEST FUNCTIONS ------------------------------- */

char *remove_all(dict_t *root, dict_item_t nodes[]) {
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		dict_item_t *deleted = avl_delete(root, &nodes[i]);
		TEST_FAIL_IF(deleted != NULL && comparator(deleted, &nodes[i]) != 0);
		TEST_FAIL_IF(avl_contains(root, &nodes[i]));
	}
	return NULL;
}

char *insert_random(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	fill_random(nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		dict_item_t *replaced = avl_insert(root, &nodes[i]);
		TEST_FAIL_IF(replaced != NULL && comparator(replaced, &nodes[i]) != 0);
		TEST_FAIL_IF(!avl_contains(root, &nodes[i]));
	}
	return NULL;
}

char *test_remove(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	return NULL;
}

char *insert_linear(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	fill_linear(nodes);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		dict_item_t *replaced = avl_insert(root, &nodes[i]);
		TEST_FAIL_IF(replaced != NULL);
		TEST_FAIL_IF(!avl_contains(root, &nodes[i]));
	}
	return NULL;
}

char *test_find(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		dict_item_t *found = avl_find(root, &nodes[i]);
		TEST_FAIL_IF(found->num != nodes[i].num);
	}
	return NULL;
}

char *test_min(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_linear(root, nodes) != NULL);
	TEST_FAIL_IF(comparator(avl_min(root), &nodes[0]) != 0);

	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);
	dict_item_t min = { .num = LONG_MAX };
	for (size_t i = 0; i < NODES_COUNT; ++i)
		if (comparator(&min, &nodes[i]) > 0)
			min = nodes[i];
	TEST_FAIL_IF(comparator(avl_min(root), &min) != 0);
	return NULL;
}

char *test_max(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_linear(root, nodes) != NULL);
	TEST_FAIL_IF(comparator(avl_max(root), &nodes[NODES_COUNT - 1]) != 0);

	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);
	dict_item_t max = { .num = LONG_MIN };
	for (size_t i = 0; i < NODES_COUNT; ++i)
		if (comparator(&max, &nodes[i]) < 0)
			max = nodes[i];
	TEST_FAIL_IF(comparator(avl_max(root), &max) != 0);
	return NULL;
}

char *test_next(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_linear(root, nodes) != NULL);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		dict_item_t *next = avl_next(root, &nodes[i]);
		TEST_FAIL_IF(next == NULL && i != NODES_COUNT - 1);
		TEST_FAIL_IF(next != NULL && next->num != (long)i + 1);
	}

	TEST_FAIL_IF(avl_next(root, &nodes[NODES_COUNT - 1]) != NULL);

	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);
	dict_item_t *nodes_copy = safe_malloc(NODES_COUNT * sizeof(dict_item_t));
	memcpy(nodes_copy, nodes, NODES_COUNT * sizeof(dict_item_t));
	qsort(nodes_copy, NODES_COUNT, sizeof(dict_item_t), comparator);
	dict_item_t *cur = avl_min(root);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
                if (i < NODES_COUNT - 1 && comparator(&nodes_copy[i], &nodes_copy[i + 1]) == 0)
                        continue;
		TEST_FAIL_IF(comparator(&nodes_copy[i], cur) != 0);
		cur = avl_next(root, cur);
	}

	free(nodes_copy);
	return NULL;
}

char *test_prev(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_linear(root, nodes) != NULL);
	for (size_t i = 0; i < NODES_COUNT; ++i) {
		dict_item_t *prev = avl_prev(root, &nodes[i]);
		TEST_FAIL_IF(prev == NULL && i != 0);
		TEST_FAIL_IF(prev != NULL && prev->num != (long)i - 1);
	}

	TEST_FAIL_IF(avl_prev(root, &nodes[0]) != NULL);

	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);
	dict_item_t *nodes_copy = safe_malloc(NODES_COUNT * sizeof(dict_item_t));
	memcpy(nodes_copy, nodes, NODES_COUNT * sizeof(dict_item_t));
	qsort(nodes_copy, NODES_COUNT, sizeof(dict_item_t), comparator);
	dict_item_t *cur = avl_max(root);
	for (size_t i = NODES_COUNT - 1; i > 0; --i) {
		if (i > 0 && comparator(&nodes_copy[i], &nodes_copy[i-1]) == 0)
			continue;
		TEST_FAIL_IF(comparator(&nodes_copy[i], cur) != 0);
		cur = avl_prev(root, cur);
	}

	free(nodes_copy);
	return NULL;
}

char *test_iterator(dict_t *root, dict_item_t nodes[]) {
	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_random(root, nodes) != NULL);

	size_t offset = root->avl_root_embed.offset;

	avl_iterator_t iter = avl_get_iterator(root, &nodes[0], &nodes[1]);

	TEST_FAIL_IF(avl_peek(root, &iter) != NULL && comparator(avl_peek(root, &iter), &nodes[0]) != 0);
	TEST_FAIL_IF(avl_peek(root, &iter) != NULL && comparator(AVL_UPCAST(iter.end, offset), &nodes[1]) != 0);

	dict_item_t *prev = avl_advance(root, &iter);
	for (dict_item_t *cur; (cur = avl_advance(root, &iter));) {
		long end = ((dict_item_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF(cur->num > end);
		TEST_FAIL_IF(comparator(prev, cur) >= 0);
		prev = cur;
	}

	iter = avl_get_iterator(root, &nodes[0], &nodes[1], AVL_DESCENDING);
	prev = avl_advance(root, &iter);
	for (dict_item_t *cur; (cur = avl_advance(root, &iter));) {
		long end = ((dict_item_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF(cur->num < end);
		TEST_FAIL_IF(comparator(prev, cur) <= 0);
		prev = cur;
	}

	dict_item_t low = { .num = 1000 };
	dict_item_t hig = { .num = 9999 };
	iter = avl_get_iterator(root, &low, &hig);
	for (dict_item_t *cur; (cur = avl_advance(root, &iter)) != NULL;) {
		long end = ((dict_item_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF(cur->num > end);
		TEST_FAIL_IF(comparator(cur, &low) < 0);
		TEST_FAIL_IF(comparator(cur, &hig) > 0);
	}

	iter = avl_get_iterator(root, &low, &hig, AVL_DESCENDING);
	prev = avl_advance(root, &iter);
	for (dict_item_t *cur; (cur = avl_advance(root, &iter));) {
		long end = ((dict_item_t *)AVL_UPCAST(iter.end, offset))->num;
		TEST_FAIL_IF(cur->num < end);
		TEST_FAIL_IF(comparator(prev, cur) <= 0);
		prev = cur;
	}

	TEST_FAIL_IF(remove_all(root, nodes) != NULL);
	TEST_FAIL_IF(insert_linear(root, nodes) != NULL);

	size_t counter = 0;
	iter = avl_get_iterator(root, &nodes[0], &nodes[NODES_COUNT - 1]);
	for (size_t i = 0; i < NODES_COUNT; ++i, ++counter)
		TEST_FAIL_IF((long)i != avl_advance(root, &iter)->num);
	TEST_FAIL_IF(counter != NODES_COUNT);

	counter = 0;
	iter = avl_get_iterator(root, &nodes[0], &nodes[NODES_COUNT - 1], AVL_DESCENDING);
	for (size_t i = NODES_COUNT; i > 0; --i, ++counter)
		TEST_FAIL_IF((long)i - 1 != avl_advance(root, &iter)->num);
	TEST_FAIL_IF(counter != NODES_COUNT);

	dict_item_t under = { .num = -100 };
	iter = avl_get_iterator(root, &under, &nodes[0]);
	TEST_FAIL_IF(avl_peek(root, &iter) != avl_min(root));

	iter = avl_get_iterator(root, &nodes[NODES_COUNT - 1], &nodes[0]);
	TEST_FAIL_IF(avl_advance(root, &iter) != NULL);

	return NULL;
}

/* --- TEST INFRASTRUCUTRE -------------------------- */

int run_test(testctx_t *ctx, dict_t *root, dict_item_t nodes[]) {
	int err_counter = 0, lasterr = 0;
	char *strerr = NULL;
	for (int i = 1; i <= ctx->repeat; ++i) {
		printf("%c%-25s%2d/%d", lasterr ? '\n' : '\r', ctx->msg, i, ctx->repeat);
		fflush(stdout);
		lasterr = false;
		if ((strerr = ctx->test(root, nodes)) != NULL) {
			printf(RED("\t%s"), strerr);
			++err_counter;
			lasterr = true;
		}
	}

	if (err_counter != 0) {
		printf("%c%-32s%s\n", lasterr ? '\n' : '\r', ctx->msg, RED("FAILED"));
		fflush(stdout);
	} else {
		puts(GREEN("\tOK"));
	}

	return err_counter;
}

int main(void) {
	srandom(time(NULL));
	dict_t root = AVL_NEW(dict_t, dict_data, comparator);
	dict_item_t *nodes = safe_malloc(NODES_COUNT * sizeof(dict_item_t));

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
	for (size_t i = 0; i < arr_len(ctxs); ++i)
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
