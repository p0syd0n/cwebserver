#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define TABLESIZE 10000000

unsigned long hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

typedef struct Bucket {
  char* key;
  char* value;
  struct Bucket* next;
  struct Bucket* previous;
} Bucket;
typedef struct Hashtable {
  size_t size;
  Bucket** table;
} Hashtable;

Hashtable* create_hashtable(size_t size) {
  Hashtable* hashtable = (Hashtable*)malloc(sizeof(Hashtable));

  Bucket** thetable = calloc(size, sizeof(Bucket*));
  hashtable->table = thetable;
  hashtable->size = size;

  return hashtable;
}

int table_add(Hashtable* hashtable, char* key, char* value) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % (hashtable->size);

  Bucket* tempbucket = (hashtable->table)[dictionary_index];
  
  if (tempbucket == NULL) {
    (hashtable->table)[dictionary_index] = (Bucket*)malloc(sizeof(Bucket));
    tempbucket = (hashtable->table)[dictionary_index];
    tempbucket->key = strdup(key);
    tempbucket->value = strdup(value);
    tempbucket->next = NULL;
    tempbucket->previous = NULL;
    return 0;
  }

  Bucket* last;
  while (tempbucket != NULL) {
    if (strcmp(tempbucket->key, key) == 0) {
      free(tempbucket->value);
      tempbucket->value = strdup(value);
      return 0;
    }
    last = tempbucket;
    tempbucket = (Bucket *)(tempbucket->next);
  }

  Bucket* next_prep = (Bucket*)malloc(sizeof(Bucket));
  last->next = next_prep;
  next_prep->key = strdup(key);
  next_prep->value = strdup(value);
  next_prep->previous = last;
  next_prep->next = NULL;
  return 0;
}

int table_remove(Hashtable* hashtable, char* key) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % (hashtable->size);
  Bucket* tempbucket = (hashtable->table)[dictionary_index];

  if (tempbucket == NULL) { errno = ENOENT; return -1; }

  while (strcmp(tempbucket->key, key) != 0) {
    if (tempbucket->next == NULL) {
      errno = ENOENT;
      return -1;
    }
    tempbucket = (Bucket *)(tempbucket->next);
  }

  if (tempbucket->previous == NULL && tempbucket->next == NULL) {
    (hashtable->table)[dictionary_index] = NULL;

  } else if (tempbucket->next != NULL && tempbucket->previous != NULL) {
    (tempbucket->next)->previous = tempbucket->previous;
    (tempbucket->previous)->next = tempbucket->next;

  } else if (tempbucket->next != NULL && tempbucket->previous == NULL) {
    (hashtable->table)[dictionary_index] = tempbucket->next;
    (tempbucket->next)->previous = NULL;

  }  else if (tempbucket->next == NULL && tempbucket->previous != NULL) {
    (tempbucket->previous)->next = NULL;

  } else {
    return -1; // tf?
  }

  free(tempbucket->key);
  free(tempbucket->value);
  free(tempbucket);



  return 0;
}

char* table_get(Hashtable* hashtable, char* key) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % (hashtable->size);

  Bucket* tempbucket = (hashtable->table)[dictionary_index];

  while (tempbucket != NULL) {
    if (strcmp(tempbucket->key, key) == 0) {
      return tempbucket->value;
    }
    tempbucket = tempbucket->next;
  }

  errno = ENOENT;
  return NULL;
}


void free_hashtable(Hashtable* hashtable) {
    for (size_t i = 0; i < hashtable->size; i++) {
        Bucket* bucket = hashtable->table[i];
        while (bucket) {
            Bucket* next = bucket->next;
            free(bucket->key);
            free(bucket->value);
            free(bucket);
            bucket = next;
        }
    }
    free(hashtable->table);
    free(hashtable);
}
