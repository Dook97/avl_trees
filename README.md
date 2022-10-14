# AVL Tree Based Generic Dictionary Library

**WARNING:** I wrote this library as an assignment during my 2nd year of uni
which is why *I discourage you* from using it for any practical applications.

## Brief Intro

The AVL Tree data structure is a flavor of the binary search tree with the
added ability of self-balancing. All basic operations are therefore guaranteed
to be $O(\log n)$, where $n$ is the number of elements inside the structure.

This specific implementation aims for a balance between generality and user
comfort which is why it's generic. However due to the C language being limited in
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

### Include the library header

```c
#include "avl.h"
```

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

Its signature has to be `int (*)(void *, void *)` and it has to return:

```
<0 if item1 < item2
 0 if item1 = item2
>0 if item1 > item2
```

For example if you already have such a comparator function `TKeyComparator`
defined on `TKey` you can do the following:

```c
int dict_compare(void *item1, void *item2) {
	return TKeyComparator(((dict_item_t *)item1)->key, ((dict_item_t *)item2)->key);
}
```

Or you could do something more interesting, like defining a lexicographical
ordering on the members of your `dict_item_t`

### Define an extractor function on `avl_node_t *`

The internal implementation of the AVL tree obviously doesn't know about your
`dict_item_t`. All it knows is the `avl_node_t` embedded in your type. For user
comfort it is therefore necessary that you define an extractor function which
returns pointer to your containng structure calculated *(through magic!)* from
a pointer to its `avl_node_t` member.

Don't worry though - the magic is already prepared for you, so all you need to
do is:

```c
void *dict_extract(avl_node_t *node) {
	return AVL_UPCAST(node, dict_item_t, avl_node);
}
```

The arguments to the `AVL_UPCAST` macro are as follows:

1. pointer to the `avl_node_t` member embedded in your struct
2. the *type* of your dictionary item structure
3. the name of the `avl_node_t` member embedded in your struct

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

---

With the helper structures ready we can *(finally)* start using the library.

### Creating new dictionary instances

To create a new dictionary instance use:

```c
dict_t dict = AVL_NEW(dict_t, dict_extract, dict_compare);
```

The arguments to the `AVL_NEW` macro are:

1. your dictionary type
2. pointer to the extractor function
3. pointer to the comparator function

### Insert

To insert `dict_item_t item` into the dictionary instance `dict_t dict` use `avl_insert`

```c
avl_insert(&dict, &item.avl_node);
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
item whose `.key == 13` we would call the `avl_find` thusly:

```c
dict_item_t seeked = { .key = 13 };
dict_item_t *found = avl_find(&dict, &seeked.avl_node);
```

`avl_find` returns a typed pointer to the found item or `NULL` if it wasn't found.

### Remove

To remove `dict_item_t item` from `dict_t dict` use `avl_remove`

```c
avl_remove(&dict, &item.avl_node);
```

`avl_remove` returns a typed pointer to the deleted item or `NULL` if the
item wasn't found in the dictionary.

### Contains

To check wheter `dict_item_t item` is present in `dict_t dict` use `avl_contains`

```c
bool item_present = avl_contains(&dict, &item.avl_node);
```

`avl_contains` returns `1` if item was found otherwise `0`

### Min

To get minimal item from `dict_t dict` use `avl_min`

```c
dict_item_t *min = avl_min(&dict);
```

`avl_min` returns a typed pointer to the minimal item in `dict`

### Max

`avl_max` shares the same interface as `avl_min`

### Next

To get the next item after `dict_item_t item` in order defined by the
comparator function use `avl_next`

```c
dict_item_t *next = avl_next(&dict, &item.avl_node);
```

`avl_next` returns a typed pointer to the next item in `dict`

### Previous

`avl_prev` shares the same interface as `avl_next`

### Iterators

An iterator facility is provided by the library.

For the following examples presume that we have a `dict_t dict` which contains
100 items whose keys span from 1 to 100.

#### Creating an iterator

There are several ways of obtaining an iterator via `avl_get_iterator`. The
arguments to this macro are as follows:

1. pointer to the dictionary structure
2. lower bound of the iterator interval
3. upper bound of the iterator interval
4. [OPTIONAL] a boolean value specifying increasing or decreasing order

If a NULL is specified in place of one of the bounds the minimum and the
maximum dictionary items will be used for the lower and upper bounds
respectively.

Use `false` as the optional fourth argument to specify decreasing order.

```c
dict_item_t lower = { .key = 13 };
dict_item_t upper = { .key = 42 };

/* get iterator over the interval 13..42 */
avl_iterator_t iterator = avl_get_iterator(&dict, &lower.avl_node, &upper.avl_node);

/* get iterator over the interval 42..13 */
avl_iterator_t reversed = avl_get_iterator(&dict, &lower.avl_node, &upper.avl_node, false);

/* get iterator over the interval 1..42 */
avl_iterator_t half_open = avl_get_iterator(&dict, NULL, &upper.avl_node);

/* get iterator over the interval 1..100 */
avl_iterator_t open = avl_get_iterator(&dict, NULL, NULL);

/* empty iterator - legal but useless */
avl_iterator_t empty = avl_get_iterator(&dict, &upper.avl_node, &lower.avl_node);
```

#### Advancing an iterator

To advance an iterator use `avl_advance`

```c
dict_item_t *next = avl_advance(&dict, &iterator);
```

`avl_advance` returns a typed pointer to the next node yielded by the
iterator. If a `NULL` is returned it means the iterator has been depleted.

The state of the iterator is modified by calling this macro. If you only wish to
get the next item without advancing the iterator use `avl_peek` which otherwise
shares the same interface as `avl_advance`

#### Note on iterator invalidation

If the underlying dictionary gets modified after an iterator was created, the
iterator is considered invalidated and any operations performed on it have an
undefined result.
