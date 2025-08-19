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


int print_bucket(Bucket* bucket) {
  printf("Printing:\n");

  printf("Key: %s\nValue: %s\nnext: %s\nprevious:%s\n", bucket->key, bucket->value, (char*)bucket->next, (char*)bucket->previous);
  return 0;
}

Bucket** create_hashtable() {
  Bucket** hashtable = calloc(TABLESIZE, sizeof(Bucket*));
  printf("Hashtable starts at: %p\n", (void*)hashtable);
  return hashtable;
}

int table_add(struct Bucket** hashtable, char* key, char* value) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % TABLESIZE;

  Bucket* tempbucket = hashtable[dictionary_index];
  
  if (tempbucket == NULL) {
    hashtable[dictionary_index] = (Bucket*)malloc(sizeof(Bucket));
    tempbucket = hashtable[dictionary_index];
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

int table_remove(Bucket** hashtable, char* key) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % TABLESIZE;
  Bucket* tempbucket = hashtable[dictionary_index];

  if (tempbucket == NULL) { errno = ENOENT; return -1; }

  while (strcmp(tempbucket->key, key) != 0) {
    if (tempbucket->next == NULL) {
      errno = ENOENT;
      return -1;
    }
    tempbucket = (Bucket *)(tempbucket->next);
  }

  if (tempbucket->previous == NULL && tempbucket->next == NULL) {
    hashtable[dictionary_index] = NULL;

  } else if (tempbucket->next != NULL && tempbucket->previous != NULL) {
    (tempbucket->next)->previous = tempbucket->previous;
    (tempbucket->previous)->next = tempbucket->next;

  } else if (tempbucket->next != NULL && tempbucket->previous == NULL) {
    hashtable[dictionary_index] = tempbucket->next;
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

char* table_get(Bucket** hashtable, char* key) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % TABLESIZE;

  Bucket* tempbucket = hashtable[dictionary_index];

  while (tempbucket != NULL) {
    if (strcmp(tempbucket->key, key) == 0) {
      return tempbucket->value;
    }
    tempbucket = tempbucket->next;
  }

  errno = ENOENT;
  return NULL;
}