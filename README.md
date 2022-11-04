# AVL Tree Based Generic Dictionary Library

**WARNING:** I wrote this library as an assignment during my 2nd year of uni
which is why *I discourage you* from using it for any practical applications.

## Brief Intro

The AVL Tree data structure is a flavor of the binary search tree with the
added ability of self-balancing. All basic operations are therefore guaranteed
to be $O(\log n)$, where $n$ is the number of elements inside the structure.

This implementation is generic. However due to the C language being limited in
this regard I had to use some non-standard language extensions. The library
compiles with `GCC v12.2.0` and `clang v14.0.6`. Other options were not tested.

## Note on memory management

This library **does not** provide any memory handling facilities - that is left
completely up to the user.

This means more work when using the library but also greater flexibility and
possibly efficiency, which is a very C-spirited tradeoff to make I think 🙂

## Supporting Structures

Before you can start using the library there are some supporting structures
which you need to create.

### Define a dictionary item type

The type can contain any members you wish and it has to contain *(at least)*
one `avl_node_t` member. If you wish to embed the structure in more than one
AVL tree it has to contain one `avl_node_t` member for each.

```c
typedef struct {
	TKey key;
	TValue value;
	avl_node_t avl_node;
} dict_item_t;
```

### Define a comparator function on `dict_item_t`

Its signature has to be `int (*)(const void *, const void *)` and it has to return:

```
<0 if item1 < item2
 0 if item1 = item2
>0 if item1 > item2
```

For example if you already have such a comparator function `TKeyComparator`
defined on `TKey` you can do the following:

```c
int dict_compare(const void *item1, const void *item2) {
	return TKeyComparator(((dict_item_t *)item1)->key, ((dict_item_t *)item2)->key);
}
```

Or you could do something more interesting, like defining a lexicographical
ordering on the members of your `dict_item_t`

### Define a dictionary type

Simply call:

```c
AVL_DEFINE_ROOT(dict_t, dict_item_t);
```

The arguments to this macro are:

1. the name of the type which will represent your dictionary
2. the name of the type of a dictionary item

The `dict_t` now represents the type of your dictionary. Of course you can use
any type name you want - it doesn't have to be `dict_t`

## Interface

With the helper structures ready we can start using the library.

### Creating new dictionary instances

To create a new dictionary instance use:

```c
dict_t dict = AVL_NEW(dict_t, avl_node, dict_compare);
```

The arguments to the `AVL_NEW` macro are:

1. your dictionary type
2. name of your `avl_node_t` member
3. pointer to your comparator function

### Insert

To insert `dict_item_t item` into the dictionary instance `dict_t dict` use `avl_insert`

```c
dict_item_t *replaced = avl_insert(&dict, &item);
```

`avl_insert` places the item inside the dictionary **potentially replacing**
any item defined equal by the comparator function.

It returns a typed pointer - in this case `dict_item_t *` - to the
replaced item or `NULL` if no item was replaced.

### Find

To find `dict_item_t item` in `dict_t dict` use `avl_find`

You need to create a `dict_item_t` instance with the members used by the
comparator function set to the values you're looking for.

So if we use `dict_item_t` as an example and presume that we're looking for an
item whose `.key == 13` we would call `avl_find` thusly:

```c
dict_item_t item = { .key = 13 };
dict_item_t *found = avl_find(&dict, &item);
```

`avl_find` returns a typed pointer to the found item or `NULL` if it wasn't found.

### Delete

To delete an item from `dict_t dict` use `avl_delete`

Similarly to `insert` you need to create a helper `dict_item_t` instance and
initialize the fields used by your comparator function.

Again assuming we want to delete an item whose `.key == 13`:

```c
dict_item_t item = { .key = 13 };
dict_item_t *deleted = avl_delete(&dict, &item);
```

`avl_delete` returns a typed pointer to the deleted item or `NULL` if the
item wasn't found in the dictionary.

### Contains

To check wheter `dict_item_t item` is present in `dict_t dict` use `avl_contains`

```c
bool item_present = avl_contains(&dict, &item);
```

`avl_contains` returns `1` if item was found otherwise `0`

### Min

To get minimal item from `dict_t dict` use `avl_min`

```c
dict_item_t *min = avl_min(&dict);
```

`avl_min` returns a typed pointer to the minimal item in `dict`

### Max

`avl_max` is used analogously to `avl_min`

### Next

To get the next item after `dict_item_t item` in order defined by the
comparator function use `avl_next`

```c
dict_item_t *next = avl_next(&dict, &item);
```

`item` doesn't have to be in the dictionary in order for `avl_next` to work.
This can be handy if you wish to find the lowest value above some threshold,
eg: *"Which of our cutomers have recently turned 40?"*

`avl_next` returns a typed pointer to the next larger item in `dict`

### Previous

`avl_prev` is used analogously to `avl_next`

## Iterators

An iterator facility is provided by the library.

### Creating an iterator

There are several ways of obtaining an iterator via `avl_get_iterator`. The
arguments to this macro are as follows:

1. pointer to the dictionary structure
2. lower bound of the iterator interval
3. upper bound of the iterator interval
4. **[OPTIONAL]** a boolean value specifying increasing or decreasing order

If a `NULL` is specified in place of one of the bounds the minimum and the
maximum dictionary items will be used for the lower and upper bounds
respectively.

Use `false` as the optional fourth argument to specify decreasing order.

```c
dict_item_t lower = { .key = 13 };
dict_item_t upper = { .key = 42 };

/* get iterator over the interval 13..42 */
avl_iterator_t iterator = avl_get_iterator(&dict, &lower, &upper);

/* get iterator over the interval 42..13 */
avl_iterator_t reversed = avl_get_iterator(&dict, &lower, &upper, false);

/* get iterator over the interval 1..42 */
avl_iterator_t half_open = avl_get_iterator(&dict, NULL, &upper);

/* get iterator over the interval 1..100 */
avl_iterator_t open = avl_get_iterator(&dict, NULL, NULL);

/* empty iterator - legal but useless */
avl_iterator_t empty = avl_get_iterator(&dict, &upper, &lower);
```

### Advancing an iterator

To advance an iterator use `avl_advance`

```c
dict_item_t *next = avl_advance(&dict, &iterator);
```

`avl_advance` returns a typed pointer to the next node yielded by the
iterator. If a `NULL` is returned it means the iterator has been depleted.

The state of the iterator is modified by calling this macro. If you only wish to
get the next item without advancing the iterator use `avl_peek` which otherwise
shares the same interface as `avl_advance`

### Note on iterator invalidation

If the underlying dictionary gets modified after an iterator was created, the
iterator is considered invalidated and any operations performed on it have an
undefined result.

---

## Implementation Reader's Guide

We're done describing the interface of the library. The following section is
written in an effort to help with understanding of the internal implementation.
It is presumed that you are familiar with the idea behind Binary Search Trees
(BSTs from now on) and their extension - AVL trees. If you are not, I'd advise
reading up on that first.

### AVL node & root

`avl_node_t` and `avl_root_t` are the two fundamental concepts used. Node
represents an entry in the tree and root represents the tree itself.

A node stores pointers to its father and its two sons. Any of them can be `NULL`.
It also stores a sign which equals: `depth of right subtree - depth of left
subtree`. The sign of a node can only be -1, 0 or 1 as per the AVL tree
invariant. Later it will be described how this information is used to aid in
keeping the tree balanced and how the sign itself is updated.

A root stores pointer to the root node as well as to the comparator function,
which is defined by the user and is used as a black box by the library. The
last member is the in-memory offset from the user defined struct to its
`avl_node_t` member. This is necessary in order to support the generic nature of
the library, but more on that later.

### Find

The basic find operation is carried out by the `avl_find_getaddr` function. Its
implementation is a trivial aplication of the BST invariant, but you might be
surprised by the double pointers. These are *pointers to father's pointer to
node*. Dealing with pointers in this way is very convenient, because it allows
us to avoid having to deal with the root node as a special case when setting
the `root_node` pointer inside `avl_root_t` and for the rest of the nodes it means
we don't have to figure out which of the two son pointers we want to modify as
we would be forced to do, if we only had a direct pointer to the node.

The function also has a nice proprety that it returns pointer to the last
node visited during the search operation. Due to the nature of BSTs this is
also the place where that node would be if we were to insert it.

`avl_find_getaddr` is then wrapped in `avl_find_impl` to provide a simple and
clean interface for the user.

### Insert

Is carried out by `avl_insert_impl`. The heavy lifting is done by the find and
balance functions.

First `avl_find_getaddr` finds the position where the node should be inserted.
Either an equal node was found in which case it is replaced by the new one and
no other operations are necessarry or it isn't in which case the node is
inserted as a leaf and `balance` has to be called to preserve the AVL invariant
(and thus keep the tree balanced).

### Delete

Is similar in some ways to insert, but it's a bit more complicated as we have
to distinguish between three cases:

1. The node is a leaf -> easy; simply remove it
2. The node has one son -> also easy; contract an edge
3. The node has both sons -> bit harder; it has to be replaced by another node

In the code you can see the first two cases can be nicely handled as a single
case. In the last case we use the minimal node from the right subtree to
replace the node.

`balance_start` is the node from which the `balance` operation will start
propagating upwards. In the first two cases this is simply the father of the
deleted node and in the third case it is the father of the node which has been
used as a replacement for the deleted node.

Just in case you're wondering about this line:

```c
balance_start = (compare_nodes(root, (*min)->father, key_node) != 0) ? (*min)->father : *min;
```

The ternary is there for the eventuality, that the minimal node from the right
subtree is also the right son of the deleted node. In that case we would be
calling `balance` on the deleted node which wouldn't exactly work well.

### Balance

Fair warning: This is probably the most cryptic part of the core library code. Brace yourselves :p

The balance operation encompasses everything that needs to be done to preserve the AVL invariant.

It is called upon the root of the subtree which has been modified by insert or
delete, does *stuff* and then continues upwards as long as needed.

The arguments are:

1. `node` - the father of the deleted/inserted node
2. `root`
3. `from_left` - signifies whether the 'signal' about change came from the left or right subtree
4. `after_delete` - signifies whether `balance` is being called after delete or insert

The function might be difficult to read because it encompasses all possible
eventualities which could arise after both insert and delete. But I simply
couldn't bring myself to separate it into more functions, because these would
be very similar and non-general which I find very ugly.

Now let's take it line by line:

```c
while (node != NULL)
```

Stop propagating upwards when you've finished balancing root. This works
because root nodes's father pointer is always `NULL`.


```c
bool newbool = after_delete ^ !from_left;
```

Probably the least descriptive var name imaginable, but I really couldn't come
up with anything better. Basically through merging the separate implementations
of the function it came to light that this value comes in handy several times.

```c
node->sign += (newbool ? +1 : -1);
```

Increase or decrease `sign` according to the 'direction' and type
(insert/delete) of the recieved signal.

```c
if (ABS(node->sign) == after_delete)
	return;
```

If you break down all the possible cases you will find that a signal never
propagates further upwards if:

* after delete `node`'s sign is +/-1
* after insert `nodes`'s sign is 0

```c
if (ABS(node->sign) == 2) {
	...
}
```

If the AVL invariant no longer holds for the current subtree, do *something*
about it. The inner block simply carries out the necessary rotations based on
the analysis of all possible cases.

```c
from_left = new_left;
node = father;
```

Move up to father and repeat the same process.

### Rotate

`rotate` is an implementation of the edge rotation operation. It serves to
reorder a subtree in a certain way which, when used properly, can restore the
AVL invariant after insert or delete.

A graphical representation of the operation:

```
     |           |
     y           x
    / \         / \
   x   C  <->  A   y
  / \             / \
 A   B           B   C
```

First we update the signs as if the edge had already been rotated. The code
behind the implementation of that might seem cryptic, but it's not really.
Basically all it does is it calculates the height of `A` and `B` relative to
the height of `C` and then calculates the resulting signs from that.

Next we change the pointers to rotate the edge.

---

With this we have concluded the description of the core mechanisms of the
library. What follows is description of some of the functionality added on top.

### Min/Max

Internally implemented by `minmax_of_subtree` and externally represented by `avl_minmax_impl`.

The implementation is very simple. Go all the way to the left to get the minimum and vice versa.

### Prev/Next

Implemented by `get_closest_node` and externally represented by `avl_prevnext_impl`.

`get_closest_node` returns the searched-for node itself or the node closest to
it in the ordering defined by comparator function. Whether it's the closest
lower or higher node is determined by the last argument to the function.

The implementation is basically a modified find which only returns the closest
node visited along the way.

The internal implementation returns the searched for node if it is found while
the public wrapper calls the internal `prevnext` function on the result if
that's the case.


### Iterators

`avl_get_iterator` creates a new iterator based on the input parameters. The
implementation is pretty straightforward. We look for the closest higher node
to the lower bound and similarly for the upper bound. Then check the validity
of the resulting interval and potentially invalidate the iterator by setting
the current pointer to `NULL`.

`avl_advance_impl` gets the next item from the iterator and advances it. Again
the implementation is straightforward.

`avl_prevnext_impl` simply returns the current item from the iterator without
modifying its state.

---

With this we have finished description of the library core implementation.
Follows a description of the implementation of the generic interface.

## Generic Interface Implementation

The generic interface of this library depends heavily on macros, a few clever
tricks and also some non-standard GNU extensions. Follows an overview of these.

### Up/Down-casting

The core mechanism through which we are able to provide a generic interface is
up/down-casting. That is: we compute the in-memory offset from the beggining of
users own struct to its `avl_node_t` member. Then, when we need to convert
`avl_node_t` to the wrapper or the reverse we can simply add/subtract the
offset from the given address and get what we need.

The offset is calculated upon invoking the `AVL_NEW` macro and stored inside
the `avl_root_t` embedded inside the user defined root type.

The calculation of the offset is defined by the `AVL_GET_MEMBER_OFFSET` macro
and the up/down-casting functionality is provided by `AVL_UPCAST` and
`AVL_DOWNCAST` respectively.

### `AVL_DEFINE_ROOT`

This macro is needed in order to define a type which stores information about
the type of the user's wrapper struct. This type information is then used to
provide the user with a typed pointer instead of `void *` as a return value.
The type also contains an `avl_root_t` member under a hardcoded name which makes
it accessible by the other macros.

The type information is stored via a typed array of length 0, which means zero
cost at runtime.

### `AVL_INVOKE_FUNCTION`

This macro calls a function with return type `avl_node_t *` and upcasts its
result to the wrapper struct.

### The Rest

The rest of the macros are simple generic wrappers around implementation
functions which ensure all arguments are safely downcasted and passed to
`AVL_INVOKE_FUNCTION`
