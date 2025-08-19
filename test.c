// #include <stdio.h>
// #include "src/hashtable.c"

// int main() {
//   unsigned long example = hash_djb2("hello");
//   printf("%lx\n", example);

//   Hashtable* hashtable = create_hashtable(100000);

//   printf("Hashtable start address: %p\n", (void*)hashtable);
//   table_add(hashtable, "key", "value");
//   table_add(hashtable, "beans", "cheese");


//   char* value = table_get(hashtable, "key");
//   char* value2 = table_get(hashtable, "beans");

//   printf("Value: %s, %s\n", value, value2);

//   int remove = table_remove(hashtable, "beans");
//   printf("removed: %d\n", remove);
//   printf("removed\n");
//   char* value3 = table_get(hashtable, "beans");

//   table_remove(hashtable, "bedafasfsfgfsg");

// }


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "src/hashtable.c" // your library

#define ENTRIES 5000000
#define SIZE 10000000

int main() {
    srand(time(NULL));
    Hashtable* table = create_hashtable(SIZE);

    int inserts = 0;
    int gets = 0;
    int updates = 0;
    int removals = 0;

    // Insert ENTRIES random key-value pairs
    for (int i = 0; i < ENTRIES; i++) {
        char key[32];
        char value[32];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);

        if (table_add(table, key, value) == 0) {
            inserts++;
        }
    }

    printf("Inserted\n");

    // Get half of them (DON’T free results here)
    for (int i = 0; i < ENTRIES / 2; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key%d", rand() % ENTRIES);

        char* result = table_get(table, key);
        if (result != NULL) {
            gets++;
            // Do not free(result) here!
            // If you want, you can store results in an array to reuse later.
        }
    }
    printf("Getted\n");

    // Update some entries
    for (int i = 0; i < ENTRIES / 4; i++) {
        char key[32];
        char value[32];
        snprintf(key, sizeof(key), "key%d", rand() % ENTRIES);
        snprintf(value, sizeof(value), "updated%d", i);

        if (table_add(table, key, value) == 0) {
            updates++;
        }
    }
    printf("Updated\n");


    // Remove some entries
    for (int i = 0; i < ENTRIES / 4; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key%d", rand() % ENTRIES);

        if (table_remove(table, key) == 0) {
            removals++;
        }
    }

    printf("removed\n");

    printf("Stress Test Summary:\n");
    printf("Inserts: %d\n", inserts);
    printf("Gets: %d\n", gets);
    printf("Updates: %d\n", updates);
    printf("Removals: %d\n", removals);

    // At the very end, you may want to free the hashtable
    // free_hashtable(table);  <-- only if you’ve written that function

    return 0;
}
