# AVL Tree

Is a self-balancing binary search tree data structure.

I'd advise against using this implementation for... anything really since I
wrote it during my first year of uni for an algorithms and data structures
class.

If you wish to try it out please see User's manual down below. If you wish to
read the source code see Reader's manual under that.

Author: Jan Doskočil

## User's manual

Here we create a new tree and some nodes which we also insert into the
structure right away. Keys have the `uint32_t` type which is aliased to
`avl_key_t`. If a node with given key is already in the structure it will be
replaced by the new node and a pointer to the replaced node will be returned.
Otherwise new node is inserted and `NULL` returned.

```c
avl_root_t root = { .root_node = NULL };
avl_node_t nodes[20];
for (size_t i = 0; i < sizeof(nodes) / sizeof(nodes[0]); ++i) {
	nodes[i] = (avl_node_t){ .key = i };
	avl_insert(&nodes[i], &root);
}
```

Memory management in regards to the nodes is left to the user, which is why the
delete function returns a pointer to the deleted node. If no node with given
key is found `NULL` is returned.

```c
avl_key_t key_of_node_to_delete = 5;
avl_node_t *deleted = avl_delete(key_of_node_to_delete, &root);
/* now you can free your memory */
if (deleted != NULL)
	free(deleted);
```

Find shares the same interface. It returns pointer to the node with given key
or `NULL`, if it wasn't found.

```c
avl_key_t key_of_node_to_find = 5;
avl_node_t *found = avl_find(key_of_node_to_find, &root);
```

## Reader's manual

Below I describe the non-trivial constructs in the source code.

### `avl_root_t`

Represents the 'root' of the tree. Holds a single pointer to the root node. The
root node does NOT store a pointer back to the structure as it's father is
always `NULL`.

### `avl_node_t`

Represents a node. A node holds it's key, sign and pointers to it's two sons
and to it's father.

The 'sign' of a node is a number used for balancing the tree so that the AVL
Tree invariant holds. If the tree is properly balanced, each node's sign can be
either -1, 0 or 1. It's equal to the difference: `depth(right_subtree) -
depth(left_subtree)` though this is not how it's calculated or maintained in
this implementation. More on that below.

### `avl_find_getaddr`

Is an internal implementation of the find operation. It recieves key of the
seeked node, pointer to the root struct and  a pointer to a variable in which a
pointer to father's pointer to the seeked node is stored.

Having this pointer is very handy in several places, because it allows us to
avoid dealing with root node edge cases and also having to figure out which of
the two son pointers belongs to our node.

It returns 1 if node with given key was found otherwise 0.

### `rotate`

Is an implementation of the edge-rotation operation which is used to balance
the tree upon insert or delete. It also updates the signs of the two nodes
involved accordingly.

It takes pointer to father's pointer to the node which is closer to root and a
boolean which determines whether the rotation is left-to-right or the reverse.

The double rotation operation is not explicitly implemented as it can be
simulated by two calls to `rotate`.

### `balance`

Is called after an insert or delete operation. It traverses the tree, updates
signs of affected nodes and carries out any rotations necessary to preserve the
AVL Tree invariant.

It takes pointer to the father of the inserted/deleted node (as that is the
first node affected), pointer to the root struct, a boolean which informs from
which of the two subtrees the 'signal' came and a boolean which informs whether
we are balancing after a delete or an insert.

The code might seem somewhat cryptic, but really it's just a condensed and
generalized version of writing out all the appropriate ifs for all possible
events.

```c
if (ABS(node->sign) == after_delete)
	return;
```

If you look at the possible cases for insert and delete you'll find that for
insert propagation of the 'signal' about depth increase always stops when a
node's new sign equals 0. Similarly for delete it ends when the sign equals
plus or minus 1. Also in these cases no balancing is necessary, so we quit the
balance function as no higher nodes will be affected.

```c
avl_node_t *father = node->father;
int new_left = (father != NULL && node == father->left_son);
```

Temporary variables stored before the rotations, which are inevitably
destructive to the structure of the tree. We need these for the next iteration
of the while loop - father will become the new 'node' and new_left will become
new 'from_left'.

```c
if (ABS(node->sign) == 2)
	...
```

This is the only case when we need to carry out any rotations. The inner block
is simply condensed version of all the ifs which would describe each of the
possible situations.

```c
from_left = new_left;
node = father;
```

Sets us up for the next node which needs balancing and sign-updating.

The loop ends either with one of the two `return`s or when we finish with the
root node, whose father pointer is always `NULL`.

### `avl_delete`

Interface is described in the User's manual.

`balance_start` is the node from which the `balance` function will start. If
`node` has 0 or 1 son it is trivially the father of the deleted node, if it has
two sons it is the father of node with the lowest key in the right subtree
under `node`.

`from_left` informs which son was deleted - left or right. This is needed for
the `balance` function.

`replace_node` is just a readability measure. It hides all the tedious
reorganizing of pointers which occurs when we wish to delete a node with two
sons. It's realized by replacing the to-be-deleted node with minimal node (that
is node with lowest key) from it's right subtree.
