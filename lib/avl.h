#ifndef avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60
#define avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60

#include <stddef.h>
#include <stdbool.h>

/* --- TYPES -------------------------------------------------- */

/* internal structure storing the information necessary for proper function of the
 * AVL tree data structure */
typedef struct avl_node {
	struct avl_node *sons[2]; // { left_son, right_son }
	struct avl_node *father;
	int sign; // right subtree depth - left subtree depth
} avl_node_t;

/* A comparator function intended for structs wrapping avl_node. Arguments are
 * expected to be non-null.
 *
 * returns <0 if item1 < item2
 * returns  0 if item1 = item2
 * returns >0 if item1 > item2
 */
#if defined(__clang__)
typedef int (*avl_comparator_t)(const void * _Nonnull item1, const void * _Nonnull item2);
#elif defined(__GNUC__)
typedef int (*avl_comparator_t)(const void *item1, const void *item2) __attribute__((nonnull));
#else
typedef int (*avl_comparator_t)(const void *item1, const void *item2);
#endif

/* internal structure representing root of the AVL tree */
typedef struct {
	avl_node_t *root_node;
	avl_comparator_t cmp;
	size_t offset; // offset from avl_node to its wrapper struct
} avl_root_t;

typedef struct {
	avl_node_t *cur, *end;
	avl_root_t *root;
	bool low_to_high;
} avl_iterator_t;

/* --- CONSTANTS ---------------------------------------------- */

/* optional last argument to avl_get_iterator which determines the iteration order */
#define AVL_ASCENDING	true
#define AVL_DESCENDING	false

/* argument to avl_minmax_impl */
#define AVL_MAX		true
#define AVL_MIN		false

/* argument to avl_prevnext_impl */
#define AVL_NEXT	true
#define AVL_PREV	false

/* --- INTERNAL FUNCTIONS ------------------------------------- */

/* returns pointer to node with given key or NULL if it wasn't found */
avl_node_t *avl_find_impl(avl_node_t *key_node, avl_root_t *root);

/* if a node with given key already existed in the tree it is replaced by
 * new_node and the pointer to it is returned, otherwise the node is inserted
 * and NULL is returned */
avl_node_t *avl_insert_impl(avl_node_t *new_node, avl_root_t *root);

/* returns pointer to deleted node or NULL if it wasn't found */
avl_node_t *avl_delete_impl(avl_node_t *key_node, avl_root_t *root);

/* get minimal or maximal node according to the ordering specified by the comparator function */
avl_node_t *avl_minmax_impl(avl_root_t *root, bool max);

/* get previous or next node according to the ordering specified by the comparator function */
avl_node_t *avl_prevnext_impl(avl_root_t *root, avl_node_t *key_node, bool next);

/* get new iterator */
avl_iterator_t avl_get_iterator_impl(avl_root_t *root, avl_node_t *lower_bound, avl_node_t *upper_bound, bool low_to_high);

/* get next item from iterator */
avl_node_t *avl_advance_impl(avl_iterator_t *iterator);

/* get next node from iterator without changing its state */
avl_node_t *avl_peek_impl(avl_iterator_t *iterator);

/* --- INTERNAL MACROS ---------------------------------------- */

/* get number of args in __VA_ARGS__ */
#define AVL_GET_ARGS_COUNT(...) (sizeof((int[]){__VA_ARGS__}) / sizeof(int))

/* get the in-memory offset of avl_node_t member from the beggining of it's wrapping struct */
#define AVL_MEMBER_OFFSET(wrapper_type, avl_member_name) \
	((size_t)&((wrapper_type *)0)->avl_member_name)

/* upcast from struct member to its wrapper struct */
#define AVL_UPCAST(ptr_to_avl_member, offset) \
	({ \
		avl_node_t *AVL_UPCAST_safe_ptr__ = (ptr_to_avl_member); \
		(AVL_UPCAST_safe_ptr__ == NULL) ? NULL : ((void *)AVL_UPCAST_safe_ptr__ - (offset)); \
	})

/* downcast from wrapper struct to its avl_node_t member */
#define AVL_DOWNCAST(ptr_to_wrapper, offset) \
	({ \
		void *AVL_DOWNCAST_safe_ptr__ = (ptr_to_wrapper); \
		(AVL_DOWNCAST_safe_ptr__ == NULL) ? NULL : ((void *)AVL_DOWNCAST_safe_ptr__ + (offset)); \
	})

/* calls function with return type avl_node_t* and yields its return value upcasted to the wrapper type */
#define AVL_INVOKE_FUNCTION(root, func_ptr, ...)                                              \
	({                                                                                    \
		avl_node_t *AVL_INVOKE_FUNCTION_avl_func_output__ = (func_ptr)(__VA_ARGS__);  \
		__auto_type AVL_INVOKE_FUNCTION_safe_root__ = (root);                         \
		(__typeof__(*AVL_INVOKE_FUNCTION_safe_root__->node_typeinfo__) *)(AVL_UPCAST( \
			AVL_INVOKE_FUNCTION_avl_func_output__,                                \
			AVL_INVOKE_FUNCTION_safe_root__->avl_root_embed.offset));             \
	})

/* --- USER FACING MACROS ------------------------------------- */

/* a shortcut to help user define his root struct */
#define AVL_DEFINE_ROOT(root_type_name, node_type_name) \
	typedef struct { \
		avl_root_t avl_root_embed; \
		node_type_name node_typeinfo__[0]; \
	} root_type_name

/* macro to initialize the user defined root struct */
#define AVL_NEW(root_type_name, avl_member_name, comparator)                         \
	(root_type_name) {                                                           \
		.avl_root_embed = (avl_root_t) {                                     \
			.root_node = NULL, .cmp = (comparator),                      \
			.offset = AVL_MEMBER_OFFSET(                                 \
				__typeof__(*((root_type_name *)0)->node_typeinfo__), \
				avl_member_name)                                     \
		}                                                                    \
	}

/* public wrappers around internal functions which deal with type conversions so that user doesn't have to */

#define avl_find(root, item)                                                                   \
	({                                                                                     \
		__auto_type avl_find_safe_root__ = (root);                                     \
		avl_node_t *avl_find_safe_node__ =                                             \
			AVL_DOWNCAST((item), avl_find_safe_root__->avl_root_embed.offset);     \
		AVL_INVOKE_FUNCTION(avl_find_safe_root__, avl_find_impl, avl_find_safe_node__, \
				    &avl_find_safe_root__->avl_root_embed);                    \
	})

#define avl_insert(root, item)                                                               \
	({                                                                                   \
		__auto_type avl_insert_safe_root__ = (root);                                 \
		avl_node_t *avl_insert_safe_node__ =                                         \
			AVL_DOWNCAST((item), avl_insert_safe_root__->avl_root_embed.offset); \
		AVL_INVOKE_FUNCTION(avl_insert_safe_root__, avl_insert_impl,                 \
				    avl_insert_safe_node__,                                  \
				    &avl_insert_safe_root__->avl_root_embed);                \
	})

#define avl_delete(root, item)                                                               \
	({                                                                                   \
		__auto_type avl_delete_safe_root__ = (root);                                 \
		avl_node_t *avl_delete_safe_node__ =                                         \
			AVL_DOWNCAST((item), avl_delete_safe_root__->avl_root_embed.offset); \
		AVL_INVOKE_FUNCTION(avl_delete_safe_root__, avl_delete_impl,                 \
				    avl_delete_safe_node__,                                  \
				    &avl_delete_safe_root__->avl_root_embed);                \
	})

#define avl_contains(root, item)                                                                   \
	({                                                                                         \
		__auto_type avl_contains_safe_root__ = (root);                                     \
		avl_node_t *avl_contains_safe_node__ =                                             \
			AVL_DOWNCAST((item), avl_contains_safe_root__->avl_root_embed.offset);     \
		avl_find_impl(avl_contains_safe_node__, &avl_contains_safe_root__->avl_root_embed) \
			!= NULL;                                                                   \
	})

#define avl_next(root, item)                                                                     \
	({                                                                                       \
		__auto_type avl_next_safe_root__ = (root);                                       \
		avl_node_t *avl_next_safe_node__ =                                               \
			AVL_DOWNCAST((item), avl_next_safe_root__->avl_root_embed.offset);       \
		AVL_INVOKE_FUNCTION(avl_next_safe_root__, avl_prevnext_impl,                     \
				    &avl_next_safe_root__->avl_root_embed, avl_next_safe_node__, \
				    AVL_NEXT);                                                   \
	})

#define avl_prev(root, item)                                                                     \
	({                                                                                       \
		__auto_type avl_prev_safe_root__ = (root);                                       \
		avl_node_t *avl_prev_safe_node__ =                                               \
			AVL_DOWNCAST((item), avl_prev_safe_root__->avl_root_embed.offset);       \
		AVL_INVOKE_FUNCTION(avl_prev_safe_root__, avl_prevnext_impl,                     \
				    &avl_prev_safe_root__->avl_root_embed, avl_prev_safe_node__, \
				    AVL_PREV);                                                   \
	})

#define avl_min(root)                                                               \
	({                                                                          \
		__auto_type avl_min_safe_root__ = (root);                           \
		AVL_INVOKE_FUNCTION(avl_min_safe_root__, avl_minmax_impl,           \
				    &avl_min_safe_root__->avl_root_embed, AVL_MIN); \
	})

#define avl_max(root)                                                               \
	({                                                                          \
		__auto_type avl_max_safe_root__ = (root);                           \
		AVL_INVOKE_FUNCTION(avl_max_safe_root__, avl_minmax_impl,           \
				    &avl_max_safe_root__->avl_root_embed, AVL_MAX); \
	})

#define avl_get_iterator(root, lower_bound, upper_bound, ...)                                 \
	({                                                                                    \
		bool avl_get_iterator_low_to_high__ =                                         \
			(AVL_GET_ARGS_COUNT(__VA_ARGS__) == 1) ? __VA_ARGS__ : AVL_ASCENDING; \
		__auto_type avl_get_iterator_safe_root__ = (root);                            \
		avl_node_t *avl_get_iterator_safe_lower__ = AVL_DOWNCAST(                     \
			(lower_bound), avl_get_iterator_safe_root__->avl_root_embed.offset);  \
		avl_node_t *avl_get_iterator_safe_upper__ = AVL_DOWNCAST(                     \
			(upper_bound), avl_get_iterator_safe_root__->avl_root_embed.offset);  \
		avl_get_iterator_impl(&avl_get_iterator_safe_root__->avl_root_embed,          \
				      avl_get_iterator_safe_lower__,                          \
				      avl_get_iterator_safe_upper__,                          \
				      avl_get_iterator_low_to_high__);                        \
	})

#define avl_advance(root, iterator) AVL_INVOKE_FUNCTION((root), avl_advance_impl, (iterator))

#define avl_peek(root, iterator) AVL_INVOKE_FUNCTION((root), avl_peek_impl, (iterator))

#endif
