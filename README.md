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

## Note on internals

It should be obvious, but the user is **NOT** expected to directly access data
inside the internal structures used by the library. A hellish landscape ripe
with segfaults surely awaits any programmer foolish enough to take on such
endeavour.

A less stringent warning applies to the use of any of the `*_impl` functions.
These only exist as infrastructure for the generic macros and there is little
reason to use them, besides I suppose a very minor performance gain, which
however would be payed for in increased code complexity and the loss of genericity.

## Supporting Structures

### Define a dictionary item type

The type can contain any members you wish and it has to contain *(at least)*
one `avl_node_t` member. If you wish to embed the structure in more than one
AVL tree it has to contain one `avl_node_t` member for each.

An example typedef which will be used throughout the rest of the documentation:

```c
typedef struct {
    TKey key;
    TValue value;
    avl_node_t dict_data;
} dict_item_t;
```

### Define a comparator function on `dict_item_t`

Its signature has to be `int (*)(const void *, const void *)` and it has to
return:

```
<0 if item1 < item2
 0 if item1 = item2
>0 if item1 > item2
```

An example comparator function assuming `TKey` is synonymous with `int`:

```c
int dict_compare(const void *item1, const void *item2) {
    int key1 = ((dict_item_t *)item1)->key, key2 = ((dict_item_t *)item2)->key;
    return (key1 == key2) ? 0
                          : (key1 < key2) ? -1 : 1;
}
```

### Define a dictionary type

Simply call:

```c
AVL_DEFINE_ROOT(dict_t, dict_item_t);
```

The arguments to this macro are:

1. type which will represent your dictionary
2. type of a dictionary item

`dict_t` is now the type of your dictionary.

Internally this is just a typedef, so it's ok to use in header files.

## Interface

### Creating new dictionary instances

To create a new dictionary instance use:

```c
dict_t dict = AVL_NEW(dict_t, dict_data, dict_compare);
```

The arguments to the `AVL_NEW` macro are:

1. dictionary type
2. name of the `avl_node_t` member of a dictionary item
3. pointer to a comparator function

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

To find an item in `dict_t dict` use `avl_find`

You need to create a dictionary item instance which is equal to the item you're
looking for as per your comparator function. That means setting its relevant
members.

```c
dict_item_t dummy = { .key = 13 };
dict_item_t *found = avl_find(&dict, &dummy);
```

`avl_find` returns a typed pointer to the found item or `NULL` if it wasn't found.

### Delete

To delete an item equal to `dict_item_t dummy` from `dict_t dict` use `avl_delete`

```c
dict_item_t dummy = { .key = 13 };
dict_item_t *deleted = avl_delete(&dict, &dummy);
```

`avl_delete` returns a typed pointer to the deleted item or `NULL` if the
item wasn't found in the dictionary.

### Contains

To check wheter an item equal to `dict_item_t item` is present in `dict_t dict`
use `avl_contains`

```c
bool item_present = avl_contains(&dict, &item);
```

`avl_contains` returns `true` if item was found otherwise `false` (as per `stdbool.h`)

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

### Creating an iterator

An iterator is obtained via `avl_get_iterator`. The arguments to this macro are
as follows:

1. pointer to the dictionary structure
2. lower bound of the iterator interval
3. upper bound of the iterator interval
4. **[OPTIONAL]** a macro specifying increasing or decreasing order

If a `NULL` is specified in place of one of the bounds the minimum and the
maximum dictionary items will be used for the lower and upper bounds
respectively.

Use `AVL_ASCENDING` or `AVL_DESCENDING` to specify iteration order. If you
leave the 4th argument out, ascending order is presumed.

Example presuming `dict` contains items whose keys range from 1 to 100:

```c
dict_item_t lower = { .key = 13 };
dict_item_t upper = { .key = 42 };

/* get iterator over the interval 13..42 */
avl_iterator_t iterator = avl_get_iterator(&dict, &lower, &upper);

/* get iterator over the interval 42..13 */
avl_iterator_t reversed = avl_get_iterator(&dict, &lower, &upper, AVL_DESCENDING);

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
