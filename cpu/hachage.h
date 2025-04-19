#ifndef HACHAGE_H
#define HACHAGE_H

#define TABLE_SIZE 128
#define TOMBSTONE ((void*)-1)  // Indicateur de suppression

// Structure pour représenter une entrée de la table de hachage
typedef struct hashentry {
    char *key;
    void *value;
} HashEntry;

// Structure pour représenter la table de hachage
typedef struct hashmap {
    int size;
    HashEntry *table;
} HashMap;

// Fonctions de gestion de la table de hachage
unsigned long simple_hash(const char *str);
HashMap *hashmap_create();
int hashmap_insert(HashMap* map,const char* key,void* value);
void *hashmap_get(HashMap *map, const char *key);
int hashmap_remove(HashMap *map, const char *key);
void hashmap_destroy(HashMap *map);

#endif 