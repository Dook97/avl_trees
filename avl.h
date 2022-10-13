#ifndef avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60
#define avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60

#include <stddef.h>
#include <stdbool.h>

/* --- TYPES -------------------------------------------------- */

/* internal structure storing the information necessary for proper function of the
 * AVL tree data structure */
typedef struct avl_node {
	struct avl_node *sons[2];
	struct avl_node *father;
	int sign; // right subtree depth - left subtree depth
} avl_node_t;

/* a readability measure - left & right serve as indicies into the sons member of avl_node_t */
typedef enum Son { left, right } avl_son_t;

/* a comparator function intended for structs wrapping avl_node
 *
 * returns <0 if item1 < item2
 * returns  0 if item1 = item2
 * returns >0 if item1 > item2
 */
typedef int (*comparator_t)(void *item1, void *item2);

/* extract wrapper struct from avl_node */
typedef void *(*extractor_t)(avl_node_t *node);

/* internal structure representing root of the AVL tree */
typedef struct {
	avl_node_t *root_node;
	extractor_t extractor;
	comparator_t comparator;
} avl_root_t;

typedef struct {
	avl_node_t *cur, *end;
	avl_root_t *root;
	bool low_to_high;
} avl_iterator_t;

/* --- INTERNAL FUNCTIONS ------------------------------------- */

/* returns pointer to node with given key or NULL if it wasn't found */
avl_node_t *avl_find_impl(avl_node_t *key_node, avl_root_t *root);

/* returns pointer to node closest to the one that was searched for as defined by the comparator function */
avl_node_t *avl_closest_impl(avl_node_t *key_node, avl_root_t *root);

/* if a node with given key already existed in the tree it is replaced by
 * new_node and the pointer to it is returned, otherwise the node is inserted
 * and NULL is returned */
avl_node_t *avl_insert_impl(avl_node_t *new_node, avl_root_t *root);

/* returns pointer to deleted node or NULL if it wasn't found */
avl_node_t *avl_delete_impl(avl_node_t *key_node, avl_root_t *root);

/* get minimal or maximal node according to the ordering specified by the comparator function */
avl_node_t *avl_minmax_impl(avl_root_t *root, bool max);

/* get previous or next node according to the ordering specified by the comparator function */
avl_node_t *avl_prevnext_impl(avl_node_t *node, bool next);

/* get new iterator */
void avl_newiterator_impl(avl_root_t *root, avl_node_t *lower_node, avl_node_t *upper_node,
		bool low_to_high, avl_iterator_t *out);

/* get next item from iterator */
avl_node_t *avl_advance_impl(avl_iterator_t *iterator);

/* --- INTERNAL MACROS ---------------------------------------- */

/* get number of args in __VA_ARGS__ */
#define AVL_GET_ARGS_COUNT(...) (sizeof((int[]){__VA_ARGS__}) / sizeof(int))

#define AVL_GET_MEMBER_OFFSET(wrapper_type, avl_member_name) \
	((size_t)&((wrapper_type *)0)->avl_member_name)

/* calls function with return type avl_node_t* and yields its return value upcasted to the wrapper type */
#define AVL_INVOKE_FUNCTION(root, function_name, ...) \
	({ \
		avl_node_t *__avl_func_output__ = (*(function_name))(__VA_ARGS__); \
		(__avl_func_output__ == NULL) ? NULL : \
			(__typeof__(*(root)->node_typeinfo__) *)((*((root)->AVL_EMBED_NAMING_CONVENTION.extractor))(__avl_func_output__)); \
	})

/* --- USER FACING MACROS ------------------------------------- */

/* name under which avl_root will be embedded inside the user defined root struct */
#define AVL_EMBED_NAMING_CONVENTION ___EMBED_avl_node_INSTANCE___

/* a shortcut to help user define his root struct */
#define AVL_DEFINE_ROOT(root_type_name, node_type_name) \
	typedef struct { \
		avl_root_t AVL_EMBED_NAMING_CONVENTION; \
		node_type_name node_typeinfo__[0]; \
	} root_type_name

/* upcast from struct member to its wrapper struct */
#define AVL_GETITEM(ptr_to_avl_member, wrapper_type, avl_member_name) \
	({ \
		const __typeof__(((wrapper_type *)0)->avl_member_name) *__mptr = (ptr_to_avl_member); \
		(wrapper_type *)((void *)__mptr - AVL_GET_MEMBER_OFFSET(wrapper_type, avl_member_name)); \
	})

/* macro to initialize the user defined root struct */
#define AVL_NEWROOT(root_type_name, extractor, comparator) \
	(root_type_name){ \
		.AVL_EMBED_NAMING_CONVENTION = (avl_root_t){ \
			.root_node  = NULL, \
			.extractor  = (extractor_t)(extractor), \
			.comparator = (comparator_t)(comparator) \
		} \
	}

#define avl_newiterator(root, lower_bound, upper_bound, ...) \
	({ \
		bool __low_to_high = ((AVL_GET_ARGS_COUNT(__VA_ARGS__ ) == 1) ? __VA_ARGS__ : true); \
		avl_iterator_t out; \
		avl_newiterator_impl(&(root)->AVL_EMBED_NAMING_CONVENTION, (lower_bound), (upper_bound), __low_to_high, &out); \
		out; \
	})

/* public wrappers around internal functions which upcast the result to the wrapper struct */
#define avl_find(node, root) \
	({ \
		avl_node_t *__safe_node = (node); \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_find_impl, __safe_node, &__safe_root->AVL_EMBED_NAMING_CONVENTION); \
	})

#define avl_insert(node, root) \
	({ \
		avl_node_t *__safe_node = (node); \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_insert_impl, __safe_node, &__safe_root->AVL_EMBED_NAMING_CONVENTION); \
	})

#define avl_delete(node, root) \
	({ \
		avl_node_t *__safe_node = (node); \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_delete_impl, __safe_node, &__safe_root->AVL_EMBED_NAMING_CONVENTION); \
	 })

#define avl_min(root) \
	({ \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_minmax_impl, &__safe_root->AVL_EMBED_NAMING_CONVENTION, false); \
	})

#define avl_max(root) \
	({ \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_minmax_impl, &__safe_root->AVL_EMBED_NAMING_CONVENTION, true); \
	})

#define avl_next(node, root) \
	({ \
		avl_node_t *__safe_node = (node); \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_prevnext_impl, __safe_node, true); \
	})

#define avl_prev(node, root) \
	({ \
		avl_node_t *__safe_node = (node); \
		__auto_type __safe_root = (root); \
		AVL_INVOKE_FUNCTION(__safe_root, avl_prevnext_impl, __safe_node, false); \
	})

#define avl_advance(iterator, root) \
	AVL_INVOKE_FUNCTION((root), avl_advance_impl, (iterator))

#endif
