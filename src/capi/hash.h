#ifndef HASH_H
#define HASH_H

#ifdef __cplusplus
extern "C" {
#endif

struct arg_hashtable_entry {
    void *k, *v;
    unsigned int h;
    struct arg_hashtable_entry *next;
};

typedef struct arg_hashtable {
    unsigned int tablelength;
    struct arg_hashtable_entry **table;
    unsigned int entrycount;
    unsigned int loadlimit;
    unsigned int primeindex;
    unsigned int (*hashfn)(const void *k);
    int (*eqfn)(const void *k1, const void *k2);
} arg_hashtable_t;

/**
 * @brief Create a hash table.
 *
 * @param   minsize   minimum initial size of hash table
 * @param   hashfn    function for hashing keys
 * @param   eqfn      function for determining key equality
 * @return            newly created hash table or NULL on failure
 */
void arg_hashtable_init(arg_hashtable_t *h, unsigned int minsize,
                        unsigned int (*hashfn)(const void *),
                        int (*eqfn)(const void *, const void *));

/**
 * @brief This function will cause the table to expand if the insertion would take
 * the ratio of entries to table size over the maximum load factor.
 *
 * This function does not check for repeated insertions with a duplicate key.
 * The value returned when using a duplicate key is undefined -- when
 * the hash table changes size, the order of retrieval of duplicate key
 * entries is reversed.
 * If in doubt, remove before insert.
 *
 * @param   h   the hash table to insert into
 * @param   k   the key - hash table claims ownership and will free on removal
 * @param   v   the value - does not claim ownership
 * @return      non-zero for successful insertion
 */
void arg_hashtable_insert(arg_hashtable_t *h, void *k, void *v);

/**
 * @brief Search the specified key in the hash table.
 *
 * @param   h   the hash table to search
 * @param   k   the key to search for  - does not claim ownership
 * @return      the value associated with the key, or NULL if none found
 */
void *arg_hashtable_search(arg_hashtable_t *h, const void *k);

/**
 * @brief Remove the specified key from the hash table.
 *
 * @param   h   the hash table to remove the item from
 * @param   k   the key to search for  - does not claim ownership
 */
void arg_hashtable_remove(arg_hashtable_t *h, const void *k);

/**
 * @brief Return the number of keys in the hash table.
 *
 * @param   h   the hash table
 * @return      the number of items stored in the hash table
 */
unsigned int arg_hashtable_count(arg_hashtable_t *h);

/**
 * @brief Change the value associated with the key.
 *
 * function to change the value associated with a key, where there already
 * exists a value bound to the key in the hash table.
 * Source due to Holger Schemel.
 *
 * @name        hashtable_change
 * @param   h   the hash table
 * @param       key
 * @param       value
 */
int arg_hashtable_change(arg_hashtable_t *h, void *k, void *v);

/**
 * @brief Free the hash table and the memory allocated for each key-value pair.
 *
 * @param   h            the hash table
 * @param   free_values  whether to call 'free' on the remaining values
 */
void arg_hashtable_fini(arg_hashtable_t *h /*, int free_values*/);

typedef struct arg_hashtable_itr {
    arg_hashtable_t *h;
    struct arg_hashtable_entry *e;
    struct arg_hashtable_entry *parent;
    unsigned int index;
} arg_hashtable_itr_t;

void arg_hashtable_itr_init(arg_hashtable_t *h, arg_hashtable_itr_t *itr);

// void arg_hashtable_itr_destroy(arg_hashtable_itr_t *itr);

/**
 * @brief Return the value of the (key,value) pair at the current position.
 */
extern void *arg_hashtable_itr_key(arg_hashtable_itr_t *i);

/**
 * @brief Return the value of the (key,value) pair at the current position.
 */
extern void *arg_hashtable_itr_value(arg_hashtable_itr_t *i);

/**
 * @brief Advance the iterator to the next element. Returns zero if advanced to end of table.
 */
int arg_hashtable_itr_advance(arg_hashtable_itr_t *itr);

/**
 * @brief Remove current element and advance the iterator to the next element.
 */
int arg_hashtable_itr_remove(arg_hashtable_itr_t *itr);

/**
 * @brief Search and overwrite the supplied iterator, to point to the entry matching the supplied
 * key.
 *
 * @return  Zero if not found.
 */
int arg_hashtable_itr_search(arg_hashtable_itr_t *itr, arg_hashtable_t *h, const void *k);

#ifdef __cplusplus
}
#endif

#endif // HASH_H
