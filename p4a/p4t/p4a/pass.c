#include "mapreduce.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define NUM_MAP 888
int counter;
pthread_mutex_t F_lock;
Partitioner partitioner;
int NUM_REDUCER;
int NUM_FILE;
char** file_names;
Mapper mapper;
Reducer reducer;

typedef struct val_node{
    char* value;
    struct val_node* next;
} val_node;

typedef struct key_node{
    char* key;
    val_node* head;
    struct key_node* next;
} key_node;

typedef struct partition_node {
    key_node* head;
    pthread_mutex_t lock;
} partition_node;

typedef struct entry{
    partition_node map[NUM_MAP];
    int key_num;
    pthread_mutex_t lock;
    key_node* seq;
    int curr_index;
} entry;

entry Hash_Map[64];

int comparator(const void* p, const void* q){
    char* left = ((key_node*)p)->key;
    char* right = ((key_node*)q)->key;
    return strcmp(left, right);
}

char* getter(char *key, int partition_number){
    key_node* keys = Hash_Map[partition_number].seq;
    char* value;
    while (1) {
        if (strcmp(keys[Hash_Map[partition_number].curr_index].key, key) == 0){
            if (keys[Hash_Map[partition_number].curr_index].head == NULL)
               return NULL;
            val_node* temp = keys[Hash_Map[partition_number].curr_index].head->next;
            value = keys[Hash_Map[partition_number].curr_index].head->value;
            keys[Hash_Map[partition_number].curr_index].head = temp;
            return value;
        } else {
            Hash_Map[partition_number].curr_index++;
            continue;
        }
        return NULL;
    }
}
void* MAP(void* arg){
    while (1) {
        char* file;
        pthread_mutex_lock(&F_lock);
        if(counter >= NUM_FILE){
            pthread_mutex_unlock(&F_lock);
            return NULL;
        }
        file = file_names[counter];
        counter++;
        pthread_mutex_unlock(&F_lock);
        (*mapper)(file);
    }
}

void* REDUCE(void* arg1){
    int partition_number = *(int*)arg1;
    free(arg1); 
    arg1 = NULL;
    if(Hash_Map[partition_number].key_num == 0)
        return NULL;
    Hash_Map[partition_number].seq = malloc(sizeof(key_node)*Hash_Map[partition_number].key_num);
    int count = 0;
    for (int i = 0; i < NUM_MAP; i++){
        key_node *curr = Hash_Map[partition_number].map[i].head;
        if (curr == NULL)
            continue;
        while (curr != NULL){
            Hash_Map[partition_number].seq[count] = *curr;
            count++;
            curr = curr -> next;
        }
    }

    qsort(Hash_Map[partition_number].seq, Hash_Map[partition_number].key_num, sizeof(key_node), comparator);

    for (int i = 0; i < Hash_Map[partition_number].key_num; i++){
        char *key = Hash_Map[partition_number].seq[i].key;
        (*reducer)(key, getter, partition_number);
    }

    //TODO free the data on heap
    //TODO free all the nodes
    for (int i = 0; i < NUM_MAP; i++){
        key_node *curr = Hash_Map[partition_number].map[i].head;
        if (curr == NULL)
            continue;
        while (curr != NULL){
            free(curr->key);
            curr->key = NULL;
            val_node* vcurr = curr->head;
            while (vcurr != NULL){
                free(vcurr->value);
                vcurr->value = NULL;
                val_node* temp = vcurr -> next;
                free(vcurr);
                vcurr = temp;
            }
            vcurr = NULL;
            key_node* tempK = curr -> next;
            free(curr);
            curr = tempK;
        }
        curr = NULL;
    }
    free(Hash_Map[partition_number].seq);
    Hash_Map[partition_number].seq = NULL;

    return NULL;
}
void MR_Emit(char *key, char *value){

    unsigned long partition_number = (*partitioner)(key, NUM_REDUCER);
    unsigned long map_number = MR_DefaultHashPartition(key, NUM_MAP);
    pthread_mutex_lock(&Hash_Map[partition_number].map[map_number].lock);
    key_node* temp = Hash_Map[partition_number].map[map_number].head;
    while(temp != NULL){
        if (strcmp(temp->key, key) == 0){
            break;
        }
        temp = temp->next;
    }
    //create a value node

    val_node* new_v = malloc(sizeof(val_node));

    new_v->value = malloc(sizeof(char)*20);

    strcpy(new_v->value, value);
    new_v->next = NULL;
    //if there is no existing node for same key
    if (temp == NULL){
        key_node *new_key = malloc(sizeof(key_node));
        new_key->head = new_v;
        new_key->next = Hash_Map[partition_number].map[map_number].head;
        Hash_Map[partition_number].map[map_number].head = new_key;
        new_key->key = malloc(sizeof(char)*20);
        strcpy(new_key->key, key);
        pthread_mutex_lock(&Hash_Map[partition_number].lock);
        Hash_Map[partition_number].key_num++;
        pthread_mutex_unlock(&Hash_Map[partition_number].lock);
    } else {
        //if there is existing node for same key
        new_v->next = temp->head;
        temp->head = new_v;
    }
    pthread_mutex_unlock(&Hash_Map[partition_number].map[map_number].lock);
}
unsigned long MR_DefaultHashPartition(char *key, int num_partitions){
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}
void MR_Run(int argc, char *argv[], 
        Mapper map, int num_mappers, 
        Reducer reduce, int num_reducers, 
        Partitioner partition){

    // initializing lock
    int rc = pthread_mutex_init(&F_lock, NULL);
    // check if rc is 0 or not
    assert(rc == 0);


    // assign arguments to global variables
    counter = 0;
    partitioner = partition;
    NUM_REDUCER = num_reducers;
    NUM_FILE = argc - 1;
    file_names = argv + 1;
    mapper = map;
    reducer = reduce;

    // hash initialization
    for (int i = 0; i < NUM_REDUCER; i++){
        pthread_mutex_init(&Hash_Map[i].lock, NULL);
        Hash_Map[i].key_num = 0;
        Hash_Map[i].curr_index = 0;
        Hash_Map[i].seq = NULL;
        for (int j = 0; j < NUM_MAP; j++){
            Hash_Map[i].map[j].head = NULL;
            pthread_mutex_init(&Hash_Map[i].map[j].lock, NULL);
        } 
    }

    pthread_t mapthreads[num_mappers];
    pthread_t reducethreads[num_reducers];

    // create map threads
    for (int i = 0; i < num_mappers; i++){
        pthread_create(&mapthreads[i], NULL, MAP, NULL);
    }

    // join waits for the threads to finish
    for (int i = 0; i < num_mappers; i++){
        pthread_join(mapthreads[i], NULL);
    }

    // create reduce threads
    for (int i = 0; i < num_reducers; i++){
        void* arg = malloc(4);
        *(int*)arg = i;
        pthread_create(&reducethreads[i], NULL, REDUCE, arg);
    }

    for (int i = 0; i < num_reducers; i++){
        pthread_join(reducethreads[i], NULL);
    }
}
