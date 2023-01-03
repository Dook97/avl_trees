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

/* a readability measure - left & right serve as indicies into the sons member of avl_node_t */
typedef enum avl_son_index { left, right } avl_son_index_t;

/* a comparator function intended for structs wrapping avl_node
 *
 * returns <0 if item1 < item2
 * returns  0 if item1 = item2
 * returns >0 if item1 > item2
 */
typedef int (*avl_comparator_t)(const void *item1, const void *item2);

/* internal structure representing root of the AVL tree */
typedef struct {
	avl_node_t *root_node;
	avl_comparator_t comparator;
	size_t offset; // offset from avl_node to its wrapper struct
} avl_root_t;

typedef struct {
	avl_node_t *cur, *end;
	avl_root_t *root;
	bool low_to_high;
} avl_iterator_t;

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
#define AVL_GET_MEMBER_OFFSET(wrapper_type, avl_member_name) \
	((size_t)&((wrapper_type *)0)->avl_member_name)

/* name under which avl_root will be embedded inside the user defined root struct */
#define AVL_ROOT_EMBED EMBED_avl_root_INSTANCE__

/* upcast from struct member to its wrapper struct */
#define AVL_UPCAST(ptr_to_avl_member, offset) \
	({ \
		avl_node_t *safe_ptr__ = (ptr_to_avl_member); \
		(safe_ptr__ == NULL) ? NULL : ((void *)safe_ptr__ - (offset)); \
	})

/* downcast from wrapper struct to its avl_node_t member */
#define AVL_DOWNCAST(ptr_to_wrapper, offset) \
	({ \
		void *safe_ptr__ = (ptr_to_wrapper); \
		(safe_ptr__ == NULL) ? NULL : ((void *)safe_ptr__ + (offset)); \
	})

/* calls function with return type avl_node_t* and yields its return value upcasted to the wrapper type */
#define AVL_INVOKE_FUNCTION(root, func_ptr, ...) \
	({ \
		avl_node_t *avl_func_output__ = (*(func_ptr))(__VA_ARGS__); \
		__auto_type safe_root__ = (root); \
		(__typeof__(*safe_root__->node_typeinfo__) *)(AVL_UPCAST(avl_func_output__, safe_root__->AVL_ROOT_EMBED.offset)); \
	})

/* --- USER FACING MACROS ------------------------------------- */

/* a shortcut to help user define his root struct */
#define AVL_DEFINE_ROOT(root_type_name, node_type_name) \
	typedef struct { \
		avl_root_t AVL_ROOT_EMBED; \
		node_type_name node_typeinfo__[0]; \
	} root_type_name

/* macro to initialize the user defined root struct */
#define AVL_NEW(root_type_name, avl_member_name, comparator_) \
	(root_type_name){ \
		.AVL_ROOT_EMBED = (avl_root_t){ \
			.root_node  = NULL, \
			.comparator = (comparator_), \
			.offset = AVL_GET_MEMBER_OFFSET(__typeof__(*((root_type_name *)0)->node_typeinfo__), avl_member_name) \
		} \
	}

/* public wrappers around internal functions which deal with type conversions so that user doesn't have to */
#define avl_find(root, item) \
	({ \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_node__ = AVL_DOWNCAST((item), safe_root__->AVL_ROOT_EMBED.offset); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_find_impl, safe_node__, &safe_root__->AVL_ROOT_EMBED); \
	})

#define avl_insert(root, item) \
	({ \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_node__ = AVL_DOWNCAST((item), safe_root__->AVL_ROOT_EMBED.offset); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_insert_impl, safe_node__, &safe_root__->AVL_ROOT_EMBED); \
	})

#define avl_delete(root, item) \
	({ \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_node__ = AVL_DOWNCAST((item), safe_root__->AVL_ROOT_EMBED.offset); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_delete_impl, safe_node__, &safe_root__->AVL_ROOT_EMBED); \
	})

#define avl_contains(root, item) \
	({ \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_node__ = AVL_DOWNCAST((item), safe_root__->AVL_ROOT_EMBED.offset); \
		avl_find_impl(safe_node__, &safe_root__->AVL_ROOT_EMBED) != NULL; \
	})

#define avl_next(root, item) \
	({ \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_node__ = AVL_DOWNCAST((item), safe_root__->AVL_ROOT_EMBED.offset); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_prevnext_impl, &safe_root__->AVL_ROOT_EMBED, safe_node__, true); \
	})

#define avl_prev(root, item) \
	({ \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_node__ = AVL_DOWNCAST((item), safe_root__->AVL_ROOT_EMBED.offset); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_prevnext_impl, &safe_root__->AVL_ROOT_EMBED, safe_node__, false); \
	})

#define avl_min(root) \
	({ \
		__auto_type safe_root__ = (root); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_minmax_impl, &safe_root__->AVL_ROOT_EMBED, false); \
	})

#define avl_max(root) \
	({ \
		__auto_type safe_root__ = (root); \
		AVL_INVOKE_FUNCTION(safe_root__, avl_minmax_impl, &safe_root__->AVL_ROOT_EMBED, true); \
	})

#define avl_get_iterator(root, lower_bound, upper_bound, ...) \
	({ \
		bool low_to_high__ = (AVL_GET_ARGS_COUNT(__VA_ARGS__ ) == 1) ? __VA_ARGS__ : true; \
		__auto_type safe_root__ = (root); \
		avl_node_t *safe_lower__ = AVL_DOWNCAST((lower_bound), safe_root__->AVL_ROOT_EMBED.offset); \
		avl_node_t *safe_upper__ = AVL_DOWNCAST((upper_bound), safe_root__->AVL_ROOT_EMBED.offset); \
		avl_get_iterator_impl(&safe_root__->AVL_ROOT_EMBED, safe_lower__, safe_upper__, low_to_high__); \
	})

#define avl_advance(root, iterator) \
	AVL_INVOKE_FUNCTION((root), avl_advance_impl, (iterator))

#define avl_peek(root, iterator) \
	AVL_INVOKE_FUNCTION((root), avl_peek_impl, (iterator))

#endif
