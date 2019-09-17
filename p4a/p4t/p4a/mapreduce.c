#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"

/***************************************/
/*          Hash Map Element           */
/***************************************/
#define VALUES_SIZE 0x1000

typedef struct {
    char* key;
    char** values;

    int size;
    int _size;
} hash_map_element;

hash_map_element* init_hash_map_element(char* key) {
    hash_map_element* e = malloc(sizeof(hash_map_element));
    e->values = malloc(sizeof(char*) * VALUES_SIZE);

    e->key = key;
    e->size = 0;
    e->_size = VALUES_SIZE;

    return e;
}

void free_hash_map_element(hash_map_element* e) {
    free(e->key);

    for(int i = 0; i < e->size; i++) {
        free(e->values[i]);
    }
    free(e->values);

    e->values = NULL;
    e->size = 0;
    e->_size = 0;

    free(e);
}

void expand_values(hash_map_element* e) {
    char** new_values = calloc(e->_size * 2, sizeof(char*));
    for(int i = 0; i < e->size; i++)
        new_values[i] = e->values[i];
    free(e->values);
    e->values = new_values;
    e->_size *= 2;
}

void add(hash_map_element* e, char* value) {
    if(e->size == e->_size)
        expand_values(e);

    e->values[e->size] = value;
    e->size++;
}

/***************************************/
/*         Hash Map Iterator           */
/***************************************/
typedef struct {
    hash_map_element* e;
    int pos;
} hash_map_element_iterator;

hash_map_element_iterator* get_hash_map_element_iterator(hash_map_element* e) {
    hash_map_element_iterator* it = malloc(sizeof(hash_map_element_iterator));

    it->pos = 0;
    it->e = e;

    return it;
}

void free_hash_map_element_iterator(hash_map_element_iterator* it) {
    free(it);
}

char hash_map_element_has_next(hash_map_element_iterator* it) {
    if(it->pos < it->e->size)
        return 1;
    else
        return 0;
}

char* hash_map_element_get_next(hash_map_element_iterator* it) {
    if(hash_map_element_has_next(it)) {
        return it->e->values[it->pos++];
    }
    else
        return NULL;
}

char* hash_map_element_get_key(hash_map_element_iterator* it) {
    return it->e->key;
}

/***************************************/
/*              Hash Map               */
/***************************************/
#define HASH_MAP_SIZE    0x5000
#define MAX_CHAIN_LEN    0x10
#define HASH_MAP_SUCCESS 0
#define HASH_MAP_FULL    -1
#define HASH_MAP_MEM_ERR -2

typedef struct {
    hash_map_element** elements;

    int size;
    int _size;
} hash_map;

int _hash_map_put(hash_map* map, hash_map_element* e);
int hash_map_put(hash_map* map, char* key, char* value);
int hash_map_rehash(hash_map* map);

int hash_map_get_hash_index(hash_map* map, char* key) {
    unsigned long hash = 5381;
    int c;

    while ((c = *key++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % map->_size;
}

int hash_map_hash(hash_map* map, char* key) {
    if(map->size > (map->_size / 2))
        return HASH_MAP_FULL;

    int index = hash_map_get_hash_index(map, key);

    for(int i = 0; i < MAX_CHAIN_LEN; i++) {
        // find an empty slot
        if(map->elements[index] == NULL)
            return index;
        // find the key
        if(!strcmp(map->elements[index]->key, key))
            return index;

        index = (index + 1) % map->_size;
    }

    return HASH_MAP_FULL;
}

// Double _size
int hash_map_rehash(hash_map* map) {
    hash_map_element** new_map = calloc(2 * map->_size, sizeof(hash_map_element*));
    if(new_map == NULL)
        return HASH_MAP_MEM_ERR;

    hash_map_element** old = map->elements;
    int old_size = map->_size;

    map->_size = old_size * 2;
    map->size = 0;
    map->elements = new_map;

    for(int i = 0; i < old_size; i++) {
        if(old[i] == NULL)
            continue;

        int status = _hash_map_put(map, old[i]);

        if (status != HASH_MAP_SUCCESS)
            return status;
    }

    free(old);
    return HASH_MAP_SUCCESS;
}

int _hash_map_put(hash_map* map, hash_map_element* e) {
    int index = hash_map_hash(map, e->key);
    while(index == HASH_MAP_FULL){
        if (hash_map_rehash(map) == HASH_MAP_MEM_ERR)
            return HASH_MAP_MEM_ERR;
        index = hash_map_hash(map, e->key);
    }

    map->elements[index] = e;
    map->size++;

    return HASH_MAP_SUCCESS;
}

int hash_map_put(hash_map* map, char* key, char* value) {
    int index = hash_map_hash(map, key);
    while(index == HASH_MAP_FULL){
        if (hash_map_rehash(map) == HASH_MAP_MEM_ERR)
            return HASH_MAP_MEM_ERR;
        index = hash_map_hash(map, key);
    }

    // create a new record
    if(map->elements[index] == NULL) {
        map->elements[index] = init_hash_map_element(key);
        map->elements[index]->key = key;
        add(map->elements[index], value);
        map->size++;
    } else {
        // add more value
        free(key);
        add(map->elements[index], value);
    }

    return HASH_MAP_SUCCESS;
}

hash_map_element_iterator* hash_map_get(hash_map* map, char* key) {
    int index = hash_map_get_hash_index(map, key);

    for(int i = 0; i < MAX_CHAIN_LEN; i++){
        if(map->elements[index] != NULL) {
            //find it
            if (!strcmp(map->elements[index]->key,key))
                return get_hash_map_element_iterator(map->elements[index]);
        }

        index = (index + 1) % map->_size;
    }

    return NULL;
}

hash_map* init_hash_map() {
    hash_map* map = malloc(sizeof(hash_map));

    map->elements = calloc(HASH_MAP_SIZE, sizeof(hash_map_element*));
    map->size = 0;
    map->_size = HASH_MAP_SIZE;

    return map;
}

void free_hash_map(hash_map* map) {
    for(int i = 0; i < map->_size; i++)
        if(map->elements[i] != NULL)
            free_hash_map_element(map->elements[i]);

    free(map->elements);
    free(map);
}

/***************************************/
/*          Hash Map Iterator          */
/***************************************/
typedef struct {
    int pos;
    hash_map* map;
} hash_map_iterator;

char hash_map_has_next(hash_map_iterator* it) {
    for(int i = it->pos; i < it->map->_size; i++) {
        if(it->map->elements[i])
            return 1;
    }

    return 0;
}

hash_map_element_iterator* hash_map_get_next(hash_map_iterator* it)  {
    hash_map_element_iterator* he_it = NULL;
    for(int i = it->pos; i < it->map->_size; i++) {
        if(it->map->elements[i]) {
            he_it = get_hash_map_element_iterator(it->map->elements[i]);
            it->pos = i + 1;
            break;
        }
    }

    return he_it;
}

hash_map_iterator* get_hash_map_iterator(hash_map* map) {
    hash_map_iterator* it = malloc(sizeof(hash_map_iterator));

    it->map = map;
    it->pos = 0;

    return it;
}

void free_hash_map_iterator(hash_map_iterator* it) {
    free(it);
}

/***************************************/
/*       Map Reduce Functionality      */
/***************************************/
typedef struct {
    char* key;
    hash_map_element* e;
} pair;

hash_map_element_iterator** it;

Partitioner gen_partition;
int num_partitions;

hash_map** partitions;
pair*** pair_arr;

int compare(const void* a, const void* b) {
    pair** pa = (pair**)a;
    pair** pb = (pair**)b;

    return strcmp((*pa)->key, (*pb)->key);
}

char* get_next(char* key, int partition_number) {
    return hash_map_element_get_next(it[partition_number]);
}

void MR_Emit(char *key, char *value) {
    int partition_num = gen_partition(key, num_partitions);
    hash_map_put(partitions[partition_num], strdup(key), strdup(value));
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;

    while ((c = *key++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % num_partitions;
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {
    gen_partition = partition;
    num_partitions = num_reducers;

    partitions = malloc(sizeof(hash_map*) * num_partitions);
    it = calloc(num_partitions, sizeof(hash_map_element_iterator*));

    for(int i = 0; i < num_reducers; i++)
        partitions[i] = init_hash_map();

    for(int i = 1; i < argc; i++)
        map(argv[i]);

    pair_arr = malloc(sizeof(pair**) * num_partitions);

    for(int i = 0; i < num_partitions; i++)
        pair_arr[i] = malloc(sizeof(pair**) * partitions[i]->size);

    for(int i = 0; i < num_partitions; i++) {
        int index = 0;

        hash_map_iterator* map_it = get_hash_map_iterator(partitions[i]);
        while(hash_map_has_next(map_it)) {
            hash_map_element_iterator* e_it = hash_map_get_next(map_it);
            pair_arr[i][index] = malloc(sizeof(pair));
            pair_arr[i][index]->key = e_it->e->key;
            pair_arr[i][index]->e = e_it->e;
            index++;
            free_hash_map_element_iterator(e_it);
        }

        free_hash_map_iterator(map_it);
        qsort (pair_arr[i], partitions[i]->size, sizeof(pair*), compare);
    }

    //for(int i = 0; i < m->size; i++) {
        //it = get_hash_map_element_iterator(pair_arr[i]->e);
        //reduce(pair_arr[i]->key, get_next, 0);
        //free(it);
        //it = NULL;
    //}
    for(int i = 0; i < num_partitions; i++) {
        for(int j = 0; j < partitions[i]->size; j++) {
            it[i] = get_hash_map_element_iterator(pair_arr[i][j]->e);
            reduce(pair_arr[i][j]->key, get_next, i);
            free(it[i]);
            it[i] = NULL;
        }
    }
    free(it);

    for(int i = 0; i < num_partitions; i++) {
        for(int j = 0; j < partitions[i]->size; j++) {
            free(pair_arr[i][j]);
        }
        free(pair_arr[i]);
    }
    free(pair_arr);

    for(int i = 0; i < num_reducers; i++) {
        free_hash_map(partitions[i]);
    }
    free(partitions);
    //free_hash_map_iterator(map_it);
}

/***************************************/
/*            Main Function            */
/***************************************/
#if 0
int main() {
    /* hash_element & iterator test */
    hash_map_element* e = init_hash_map_element(strdup("test!"));
    for(int i = 0; i < 0x5000; i++) {
        char* str = malloc(sizeof(char) * 0x6);
        snprintf(str, 0x6 / sizeof(char), "%d", i);
        add(e, str);
    }
    hash_map_element_iterator* it = get_hash_map_element_iterator(e);
    while(hash_map_element_has_next(it)) {
        printf("%s\n", hash_map_element_get_next(it));
    }

    free_hash_map_element(e);
    free_hash_map_element_iterator(it);

    /* hash map test */
    hash_map* map = init_hash_map();

    for(int i = 0; i < 0x5000; i++) {
        char* str = malloc(sizeof(char) * 0x10);
        snprintf(str, 0x10 / sizeof(char), "%x", i);

        hash_map_put(map, strdup(str), str);
    }

    hash_map_element_iterator* it = hash_map_get(map, "0");
    if(it == NULL) {
        printf("Error!\n");
        free_hash_map(map);

        exit(1);
    }

   if(it)
        free_hash_map_element_iterator(it);

    hash_map_iterator* map_it = get_hash_map_iterator(map);
    while(hash_map_has_next(map_it)) {
        hash_map_element_iterator* _it = hash_map_get_next(map_it);
        while(hash_map_element_has_next(_it)) {
            printf("key: %s, data: %s\n", hash_map_element_get_key(_it), hash_map_element_get_next(_it));
        }
        free_hash_map_element_iterator(_it);
    }

    free_hash_map_iterator(map_it);
    free_hash_map(map);

    return 0;
}
#endif
