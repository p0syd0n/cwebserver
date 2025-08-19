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

typedef struct Node {
  char* key; // Pointer to key string
  char* value; // Pointer to value string
  struct Node* next; // Pointer to next item in linked list
  struct Node* previous; // Pointer to previous item in linked list
} Node;

typedef struct Hashtable {
  size_t size; // Size, so we don't have to rely on a macro
  Node** table; // Pointer to Node pointer array
} Hashtable;

Hashtable* create_hashtable(size_t size) {
  Hashtable* hashtable = (Hashtable*)malloc(sizeof(Hashtable));

  Node** thetable = calloc(size, sizeof(Node*)); // Set all the nodes to NULL pointers at the beginning, to show that they are empty
  hashtable->table = thetable;
  hashtable->size = size;

  return hashtable;
}

int table_add(Hashtable* hashtable, char* key, char* value) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % (hashtable->size); // Will always return a value between 0 and size-1 inclusive

  Node* tempnode = (hashtable->table)[dictionary_index]; // This is the pointer to the head of the linked list for the given dictionary index
  
  if (tempnode == NULL) { // It's empty (there is no head. There is no leading node.). We make a new one.
    (hashtable->table)[dictionary_index] = (Node*)malloc(sizeof(Node));
    tempnode = (hashtable->table)[dictionary_index];
    tempnode->key = strdup(key); // We take a copy in case the user decides to free his instance of the variable
    tempnode->value = strdup(value); // Same thing here
    tempnode->next = NULL; // There is no next one yet
    tempnode->previous = NULL; // We are the head so there is no previous node
    return 0;
  }

  // Slightly convoluted logic to loop through the linked list of nodes with the given dictionary index
  while (tempnode != NULL) {
    // This is the update case. We found the value, it is already in the table.
    if (strcmp(tempnode->key, key) == 0) {
      free(tempnode->value); // We own this - we strduped it earlier.
      tempnode->value = strdup(value); // The new value is added
      return 0;
    }
    if (tempnode->next == NULL) break; // If we are on the last node, break out. We have not found a node with our key.

    // The previous null check case was not true, so there is another node after this one. 
    tempnode = (Node *)(tempnode->next);
  }

  // We did not find a node with the key provided. We must add one at the end of the linked list for this dictionary index.
  Node* next_prep = (Node*)malloc(sizeof(Node));
  tempnode->next = next_prep;
  next_prep->key = strdup(key); // Again, strduping to make sure we have ownership of the values in the dictionary.
  next_prep->value = strdup(value); // Otherwise, they may get free'd if the caller decides to free them. 
  next_prep->previous = tempnode;
  next_prep->next = NULL;
  return 0;
}

int table_remove(Hashtable* hashtable, char* key) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % (hashtable->size);
  Node* tempnode = (hashtable->table)[dictionary_index];

  if (tempnode == NULL) { errno = ENOENT; return -1; } // It does not exist. Thus, we create problems for the user to handle. Mwahahaha

  while (strcmp(tempnode->key, key) != 0) {
    if (tempnode->next == NULL) {
      // The last node is not it and we have not found the desired key.
      errno = ENOENT;
      return -1;
    }
    // We have not yet reached the end. We continue down the linked list.
    tempnode = (Node *)(tempnode->next);
  }


  // Are all of these really necessary?

  if (tempnode->previous == NULL && tempnode->next == NULL) {
    // There is not a node before me and there is not a node after me. I am the only node with my dictionary index.
    (hashtable->table)[dictionary_index] = NULL;

  } else if (tempnode->next != NULL && tempnode->previous != NULL) {
    // There is a node after me and a node before me.
    (tempnode->next)->previous = tempnode->previous; // Hey, node after me! (The node before you) (previously me) is now the node before me!
    (tempnode->previous)->next = tempnode->next; // Hey, node before me! (The node after you) (previously me) is now the node after me!

  } else if (tempnode->next != NULL && tempnode->previous == NULL) {
    // There is a node after me and not a node before me.
    (hashtable->table)[dictionary_index] = tempnode->next; // Set the dictionary index pointer to point at the node after me. It is the new head.
    (tempnode->next)->previous = NULL; // It does not have a node before it now. It is the head.

  }  else if (tempnode->next == NULL && tempnode->previous != NULL) {
    // There is not a node after me and there is a node before me.
    (tempnode->previous)->next = NULL; // Hey, node before me! I no longer exist.

  } else {
    return -1; // tf?
  }

  // Ritual suicide.
  free(tempnode->key);
  free(tempnode->value);
  free(tempnode);



  return 0;
}

char* table_get(Hashtable* hashtable, char* key) {
  unsigned long hash = hash_djb2(key);
  int dictionary_index = hash % (hashtable->size);

  Node* tempnode = (hashtable->table)[dictionary_index];

  while (tempnode != NULL) {
    // If its not NULL, check if the key matches.
    if (strcmp(tempnode->key, key) == 0) {
      // strdup here would be safer. But it also almost doubles time taken. The caller needs to be careful not to free the pointer they are given.
      return tempnode->value;
    }
    tempnode = tempnode->next;
  }

  // If we got to this point, we did not find a node, and broke out of the loop because the new node in the linked list was NULL.
  errno = ENOENT;
  return NULL;
}


void free_hashtable(Hashtable* hashtable) {
    for (size_t node_pointer_index = 0; node_pointer_index < hashtable->size; node_pointer_index++) {
        Node* node = (hashtable->table)[node_pointer_index];
        // Just go down the linked list freeing everybody
        while (node) {
            Node* next = node->next;
            free(node->key);
            free(node->value);
            free(node);
            node = next;
        }
    }
    free(hashtable->table);
    free(hashtable);
}
