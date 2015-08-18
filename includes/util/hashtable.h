#include <string.h>

// A function for hashing a key.
typedef unsigned int (*hashtable_hash_func)(void *key);
// A function for comparing two keys
typedef int (*hashtable_cmp_func)(void *p1, void *p2);

#define HASHTABLE_ITER_CONTINUE 1
#define HASHTABLE_ITER_STOP     0
// A function that iterates over key, value pairs.
// returns:
//  * HASHTABLE_ITER_CONTINUE to continue iteration
//  * HASHTABLE_ITER_STOP to stop iteration
typedef int (*hashtable_iter_func)(void *key, void *val);

typedef struct hashtable_entry {
    void *key;
    void *val;
    struct hashtable_entry *next;
} hashtable_entry;

typedef struct {
    int size;
    int cap;
    hashtable_entry **table;

    hashtable_hash_func hash_func;
    hashtable_cmp_func  cmp_func;
} hashtable_t;

// Initializes a hashtable, using the given hash
// function and comparison function.
void hashtable_init(hashtable_t *ht, hashtable_hash_func hf, hashtable_cmp_func cf);

// Frees memory used by the given hashtable.
void hashtable_free(hashtable_t *ht);

// Places value associated with the given key in `dst`,
// if the key is present.
// returns:
//  * 1 if the key is present
//  * 0 if the key is not present
int hashtable_get(hashtable_t *ht, void *key, void **dst);

// Associates a key with a value.
// returns:
//  * 1 if the key was already present
//  * 0 if the key was not already present
int hashtable_put(hashtable_t *ht, void *key, void *val);

// Removes and places the value associated with the
// given key in `dst`.
// returns:
//  * 1 if the key was removed
//  * 0 if the key did not exist
int hashtable_remove(hashtable_t *ht, void *key, void **dst);

// Gets the size of the hashtable.
int hashtable_size(hashtable_t *ht);

// Iterates over a hashtable, passing (key, value)
// pairs to `ifn`.
// Iteration is stopped when the given iteration
// function returns HASHTABLE_ITER_STOP (or no
// more pairs are left).
int hashtable_iter(hashtable_t *ht, hashtable_iter_func ifn);

unsigned int fnv_1a_hash(char *bytes, int len);
unsigned int fnv_1a_hash_str(char *str);

// predefined string hashing & comparison functions
#define hash_func_str ((unsigned int (*)(void *)) fnv_1a_hash_str)
#define cmp_func_str  ((int (*)(void *, void*)) strcmp)

#define _GEN_FUNC_PROTOS(type) \
    unsigned int hash_func_##type(void *ptr); \
    int cmp_func_##type(void *p1, void *p2);

_GEN_FUNC_PROTOS(int8_t)
_GEN_FUNC_PROTOS(int16_t)
_GEN_FUNC_PROTOS(int32_t)
_GEN_FUNC_PROTOS(int64_t)
 
_GEN_FUNC_PROTOS(uint8_t)
_GEN_FUNC_PROTOS(uint16_t)
_GEN_FUNC_PROTOS(uint32_t)
_GEN_FUNC_PROTOS(uint64_t)

