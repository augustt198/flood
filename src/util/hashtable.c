#include "hashtable.h"

#include <stdlib.h>

// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
#define FNV_32_OFFSET_BASIS 2166136261
#define FNV_32_PRIME        16777619

// hash `len` bytes from the `bytes` pointer
unsigned int fnv_1a_hash(char *bytes, int len) {
    unsigned int hash = FNV_32_OFFSET_BASIS;
    for (int i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= FNV_32_PRIME;
    }
    return hash;
}

// hash each byte in `str` until NUL
unsigned int fnv_1a_hash_str(char *str) {
    unsigned int hash = FNV_32_OFFSET_BASIS;
    for (; *str != '\0'; str++) {
        hash ^= *str;
        hash *= FNV_32_PRIME;
    }
    return hash;
}

#define _GEN_FUNCS(type)                            \
    unsigned int hash_func_##type(void *ptr) {      \
        type v = *((type*) ptr);                    \
        if (sizeof(type) > sizeof(unsigned int))    \
            return v ^ (v >> 32);                   \
        else                                        \
            return v;                               \
    }                                               \
    int cmp_func_##type(void *p1, void *p2) {       \
        type v1 = *((type*) p1);                    \
        type v2 = *((type*) p2);                    \
        return (v1 > v2) - (v1 < v2);               \
    }

_GEN_FUNCS(int8_t)
_GEN_FUNCS(int16_t)
_GEN_FUNCS(int32_t)
_GEN_FUNCS(int64_t)
 
_GEN_FUNCS(uint8_t)
_GEN_FUNCS(uint16_t)
_GEN_FUNCS(uint32_t)
_GEN_FUNCS(uint64_t)

#define INIT_CAP    16
#define LOAD_FACTOR 0.75

void hashtable_init(hashtable_t *ht, hashtable_hash_func hf, hashtable_cmp_func cf) {
    ht->size = 0;
    ht->cap  = INIT_CAP;

    ht->table = calloc(1, sizeof(hashtable_entry*) * INIT_CAP);

    ht->hash_func = hf;
    ht->cmp_func  = cf;
}

void hashtable_free(hashtable_t *ht) {
    for (int i = 0; i < ht->cap; i++) {
        hashtable_entry *e = ht->table[i];
        while (e != NULL) {
            hashtable_entry *next = e->next;
            free(e);
            e = next;
        }
    }
    free(ht->table);
}

int hash_function(int hash, int tablesize) {
    int ret = (hash /* * (hash + 3) */) % tablesize;
    if (ret < 0)
        ret = ret * -1;

    return ret;
}

int hashtable_get(hashtable_t *ht, void *key, void **dst) {
    int hash  = ht->hash_func(key);
    int index = hash_function(hash, ht->cap);

    hashtable_entry *e = ht->table[index];
    while (e != NULL) {
        if (ht->cmp_func(key, e->key) == 0) {
            *dst = e->val;
            return 1;
        } else {
            e = e->next;
        }
    }
    return 0;
}

void ht_insert(hashtable_entry *entry, hashtable_entry **table,
            int size, hashtable_hash_func hf) {

    int hash  = hf(entry->key);
    int index = hash_function(hash, size);

    entry->next = NULL;
    if (table[index] == NULL) {
        table[index] = entry;
    } else {
        hashtable_entry *e = table[index];
        while (e->next != NULL) {
            e = e->next;
        }
        e->next = entry;
    }
}

void grow_table(hashtable_t *ht) {
    int newcap      = ht->cap * 2; 
    hashtable_entry **newtable = calloc(1, sizeof(hashtable_entry*) * newcap);
    
    for (int i = 0; i < ht->cap; i++) {
        hashtable_entry *e = ht->table[i];
        while (e != NULL) {
            hashtable_entry *next = e->next;
            ht_insert(e, newtable, newcap, ht->hash_func);
            e = next;
        }
    }

    free(ht->table);

    ht->cap   = newcap;
    ht->table = newtable;
}

hashtable_entry *new_entry(void *key, void *val) {
    hashtable_entry *entry = calloc(1, sizeof(hashtable_entry));
    entry->key  = key;
    entry->val  = val;
    entry->next = NULL;

    return entry;
}

int hashtable_put(hashtable_t *ht, void *key, void *val) {
    if (ht->size >= ht->cap * LOAD_FACTOR) {
        grow_table(ht);        
    }

    ht->size += 1;
    
    int hash  = ht->hash_func(key);
    int index = hash_function(hash, ht->cap);

    if (ht->table[index] == NULL) {
        ht->table[index] = new_entry(key, val);
        return 0;
    } else {
        hashtable_entry *e    = ht->table[index];
        hashtable_entry *last = NULL;

        while (e != NULL) {
            if (ht->cmp_func(key, e->key) == 0) {
                e->val = val;
                return 1;
            }

            last = e;
            e = e->next;
        }
        last->next = new_entry(key, val);
        return 0;
    }
}

int hashtable_remove(hashtable_t *ht, void *key, void **dst) {
    int hash  = ht->hash_func(key);
    int index = hash_function(hash, ht->cap);

    ht->size -= 1;
    
    if (ht->table[index] == NULL) {
        return 0;
    } else {
        hashtable_entry *e    = ht->table[index];
        hashtable_entry *prev = NULL;

        while (e != NULL) {
            if (ht->cmp_func(key, e->key) == 0) {
                if (dst != NULL)
                    *dst = e->val;

                if (prev == NULL) {
                    // entry is first node
                    ht->table[index] = e->next;
                } else {
                    prev->next = e->next;
                }
                return 1;
            }

            prev = e;
            e = e->next;
        }

        return 0;
    }
}

int hashtable_size(hashtable_t *ht) {
    return ht->size;
}

int hashtable_iter(hashtable_t *ht, hashtable_iter_func ifn) {
    int count = 0;
    for (int i = 0; i < ht->cap; i++) {
        hashtable_entry *e = ht->table[i];
        while (e != NULL) {
            if (ifn(e->key, e->val) == HASHTABLE_ITER_STOP)
                goto done;
            count++;

            e = e->next;
        }
    }
    done:
    return count;
}
