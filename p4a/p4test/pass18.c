#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mapreduce.h"

#ifndef NUM_BUCKETS
		#define NUM_BUCKETS 1000
#endif

int f_index;
int end;
char **files;
pthread_mutex_t lock;
Mapper mapping;
Reducer reducing;
Partitioner partitioning;
struct outerTable hmap;
int reducerNum;

struct v_node {
	char* value;
	struct v_node *next;
};

struct node {
	char* key;
	char* value;
	int valueSum;
	int visited;
	struct node *next;
	struct v_node *v_next;
};

struct innerTable {
	struct node **list;
	int num_keys;
	pthread_mutex_t i_lock;
	char **sort;
};

struct outerTable {
	int size;
	struct innerTable **inner;
	pthread_mutex_t o_lock;
};

void hmap_init(int size) {
	pthread_mutex_init(&hmap.o_lock, NULL);
	hmap.size = size;
	hmap.inner = malloc(size * (sizeof(struct innerTable*)));
	for(int i = 0;i < size;i++) {
		hmap.inner[i] = malloc(sizeof(struct innerTable));
		hmap.inner[i]->num_keys = 0;
		pthread_mutex_init(&hmap.inner[i]->i_lock, NULL);
		hmap.inner[i]->list = malloc(NUM_BUCKETS * (sizeof(struct node*)));
		hmap.inner[i]->sort = NULL;
	}
  for(int j = 0;j < size;j++)
    for(int i = 0;i < NUM_BUCKETS;i++)
      hmap.inner[j]->list[i] = NULL;
}

// unhandled rehash size
unsigned long innerPartition(char *key) {
	unsigned long hash = 5381;
	int c;
	while ((c = *key++) != '\0')
			hash = hash * 33 + c;
	return hash % NUM_BUCKETS;
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
	unsigned long hash = 5381;
	int c;
	while ((c = *key++) != '\0')
			hash = hash * 33 + c;
	return hash % num_partitions;
}

// inserting
void MR_Emit(char *key, char *value) {	
	pthread_mutex_lock(&hmap.o_lock);
	int outer_idx = partitioning(key, reducerNum);
	int inner_idx = innerPartition(key);	
	struct node *curr = malloc(sizeof(struct node));
	curr->key = malloc(sizeof(char) * 20);
	strcpy(curr->key, key);
	curr->value = malloc(sizeof(char) * 20);
	strcpy(curr->value, value);
	curr->visited = 0;
	curr->valueSum = 0;

	if(hmap.inner[outer_idx]->list[inner_idx] == NULL) {
		pthread_mutex_lock(&hmap.inner[outer_idx]->i_lock);
		hmap.inner[outer_idx]->list[inner_idx] = malloc(sizeof(struct node));
		hmap.inner[outer_idx]->list[inner_idx] = curr;		
		hmap.inner[outer_idx]->num_keys++;
		pthread_mutex_unlock(&hmap.inner[outer_idx]->i_lock);
	}
	else {
		pthread_mutex_lock(&hmap.inner[outer_idx]->i_lock);
		int isKey = 0;
		struct node *check_key = hmap.inner[outer_idx]->list[inner_idx];
		while(check_key != NULL) {
			if(strcmp(check_key->key, key) == 0) {
				isKey = 1;
				check_key->valueSum++;
				struct v_node *temp = check_key->v_next;
				struct v_node *curr = malloc(sizeof(struct v_node));
				curr->value = malloc(sizeof(char) * 20);
				strcpy(curr->value, value);
				if(temp == NULL) {
					temp = malloc(sizeof(struct v_node));
					temp = curr;
				}
				else if(temp != NULL) {
					while(temp != NULL)
						temp = temp->next;
					temp = malloc(sizeof(struct v_node));
					temp = curr;
				}
				break;
			}
			else
				check_key = check_key->next;
		}

		if(isKey == 0) {
			struct node *temp = hmap.inner[outer_idx]->list[inner_idx];
			hmap.inner[outer_idx]->num_keys++;
			while(temp->next != NULL)
				temp = temp->next;
			temp->next = malloc(sizeof(struct node));
			temp->next = curr;
		}
		pthread_mutex_unlock(&hmap.inner[outer_idx]->i_lock);
	}
	pthread_mutex_unlock(&hmap.o_lock);
}


void* MAP(void* arg) {
	pthread_mutex_lock(&lock);
	if(f_index >= end) {
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	char *file = files[f_index];
	f_index++;
	mapping(file);	
	pthread_mutex_unlock(&lock);
	return NULL;
}

// looking
char* get_next(char *key, int partition_number) {
	int inner_idx = innerPartition(key);
	struct node *curr = hmap.inner[partition_number]->list[inner_idx];
	while(curr != NULL && strcmp(curr->key, key) != 0)
		curr = curr->next;	

	if(curr == NULL)
		return NULL;
	if(curr->visited == 1 && curr->valueSum == 0)
		return NULL;
	if(curr->visited == 1) {
		curr->valueSum--;
		char* value = curr->value;
		return value;
	}
	else {
		curr->visited = 1;
		char* value = curr->value;
		return value;
	}
	return NULL;
}

int compare_function(const void *name1, const void *name2)
{
    const char *name1_ = *(const char **)name1;
    const char *name2_ = *(const char **)name2;
    return strcmp(name1_, name2_);
}

void* REDUCE(void* arg) {
	int partition_idx = *((int*)arg);
	if(hmap.inner[partition_idx]->num_keys == 0)
		return NULL;

	// sorting
	pthread_mutex_lock(&hmap.o_lock);
	int size = hmap.inner[partition_idx]->num_keys;
	char *sort_keys[size];
	struct innerTable *partition = hmap.inner[partition_idx];	

	int index = 0;
	for(int i = 0;i < NUM_BUCKETS;i++) {
		if(partition->list[i] != NULL) {
			struct node *temp = partition->list[i];
			if(temp->next != NULL) {
				sort_keys[index] = temp->key;
				index++;
				while(temp->next != NULL) {
					sort_keys[index] = temp->next->key;
					index++;
					temp = temp->next;
				}
			}
			else {
				sort_keys[index] = temp->key;
				index++;
			}
		}
	}

	/*
	for(int i = 0;i < NUM_BUCKETS;i++) {
		if(partition->list[i] != NULL) {
			struct node *temp = partition->list[i];
			printf("key: %s\n", temp->key);
			if(temp->next != NULL) {
				while(temp->next != NULL) {
					printf("key: %s\n", temp->next->key);
					temp = temp->next;
				}
			}
			else {
					printf("key: %s\n", temp->key);
			}
		}
	}
	printf("----------------------------------------\n");
	*/

	qsort(sort_keys, size, sizeof(char*), compare_function);
	hmap.inner[partition_idx]->sort = sort_keys;

	for(int i = 0;i < size;i++)
		reducing(hmap.inner[partition_idx]->sort[i], get_next, partition_idx);	
	pthread_mutex_unlock(&hmap.o_lock);

	// freeing
	for(int i = 0;i < NUM_BUCKETS;i++) {
		if(partition->list[i] != NULL) {
			while(partition->list[i] != NULL) {
				struct node *temp;
				while(partition->list[i]->v_next != NULL) {
					struct v_node *temp = partition->list[i]->v_next;
					partition->list[i]->v_next = partition->list[i]->v_next->next;
					free(temp->value);
					temp->value = NULL;
				}
				temp = partition->list[i];
				partition->list[i] = partition->list[i]->next;
				free(temp->key);
				temp->key = NULL;
				free(temp->value);
				temp->value = NULL;
				free(temp);
				temp = NULL;
			}
			//free(partition->list);
			//partition->list = NULL;
		}
	}

	partition->sort = NULL;
	free(partition);
	partition = NULL;
	return NULL;
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {

	// globals
	f_index = 1;
	pthread_mutex_init(&hmap.o_lock, NULL);
	mapping = map;
	reducing = reduce;
	partitioning = partition;
	files = argv;
	end = argc;
	reducerNum = num_reducers;

	// initialize
	hmap_init(num_reducers);
	int fileNum = argc - 1;

	// for mapping threads
	pthread_t m_threads[num_mappers];
	if(num_mappers >= fileNum) {
		for(int i = 0;i < num_mappers;i++) {
			if(pthread_create(&m_threads[i], NULL, MAP, NULL) != 0)
				return;
		}
	}
	// RACING!!!
	else if(num_mappers < fileNum) {
		while(fileNum != 0) {
			for(int i = 0;i < num_mappers;i++) {
				if(fileNum == 0)
					break;
				if(pthread_create(&m_threads[i], NULL, MAP, NULL) != 0)
					return;
				fileNum--;
			}
			for(int i = 0;i < num_mappers;i++)
				pthread_join(m_threads[i], NULL);
		}
	}


	for(int i = 0;i < num_mappers;i++)
		pthread_join(m_threads[i], NULL);

	// for reducing threads
	pthread_t r_threads[num_reducers];
	for(int i = 0;i < num_reducers;i++) {
		int *temp = malloc(sizeof(int));
		*temp = i;
		if(pthread_create(&r_threads[i], NULL, REDUCE, temp) != 0)
			return;
	}

	for(int i = 0;i < num_reducers;i++)
		pthread_join(r_threads[i], NULL);
	
}
