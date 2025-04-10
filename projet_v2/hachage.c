#include <stdlib.h>
#include <string.h>
#include "hachage.h"
#include "logger.h"

unsigned long simple_hash(const char *str) {
    unsigned long hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++; // hash * 31 + c
    }
    return hash % TABLE_SIZE; // Utiliser TABLE_SIZE au lieu de len
}

HashMap *hashmap_create() {
    // Allocation de la table de hachage
    HashMap *hash = (HashMap*)malloc(sizeof(HashMap));
    if (hash == NULL) { 
        LOG_ERROR("Erreur d'allocation mémoire");
        return NULL;
    }
    hash->size = TABLE_SIZE;
    hash->table = (HashEntry*)malloc(sizeof(HashEntry) * TABLE_SIZE);
    if (hash->table == NULL) { 
        LOG_ERROR("Erreur d'allocation mémoire");
        free(hash);
        return NULL;
    }
    // Initialiser toutes les entrées à NULL
    for (int i = 0; i < TABLE_SIZE; i++) {
        hash->table[i].key = NULL;
        hash->table[i].value = NULL;
    }
    return hash;
}
int hashmap_insert(HashMap *map, const char *key, void *value) {
    /*On suppose que si la cle est deja associée à une valeur dans la table de hachage, alors l'ancienne valeur est remplacée par la nouvelle*/
    unsigned int index = simple_hash(key) % map->size;
    unsigned int original_index = index; // Garder l'index original pour éviter les boucles infinies
    // Chercher un emplacement libre
    while (map->table[index].key != NULL  && map->table[index].key != TOMBSTONE) {
        // Si la clé existe déjà, on peut choisir de remplacer la valeur
        if (strcmp(map->table[index].key, key) == 0) {
            // Remplacer la valeur existante
            map->table[index].value = value;
            return 1; // Indiquer que l'insertion a réussi (remplacement)
        }
        // Passer à l'emplacement suivant
        index = (index + 1) % map->size;
        // Si nous avons fait un tour complet, cela signifie que la table est pleine
        if (index == original_index) {
            return 0; // Indiquer que l'insertion a échoué 
        }
    }
    // Insérer la nouvelle entrée
    map->table[index].key = strdup(key); // Dupliquer la clé
    if (map->table[index].key == NULL) {
        return 0; //Echec de strdup
    }
    map->table[index].value = value; // Assigner la valeur
    
    return 1; // Indiquer que l'insertion a réussi
}

void *hashmap_get(HashMap *map, const char *key) {
    if (map == NULL || key == NULL) {
        LOG_ERROR("Erreur, HashMap est null ou la clé est nulle");
        return NULL;
    }
    unsigned int index = simple_hash(key) % map->size;
    unsigned int original_index = index;

    while (map->table[index].key != NULL) {
        if (map->table[index].key != TOMBSTONE && strcmp(map->table[index].key, key) == 0) {
            return map->table[index].value;
        }
        index = (index + 1) % map->size;
        if (index == original_index) {
            break; 
        }
    }
    return NULL; 
}

int hashmap_remove(HashMap *map, const char *key) {
     if (map == NULL || key == NULL) {
        LOG_ERROR("Erreur : paramètres invalides");
        return 0;
    }
    unsigned int index = simple_hash(key) % map->size;
    unsigned int index_original = index;
    while (map->table[index].key != NULL) {
        if (map->table[index].key != TOMBSTONE && strcmp(map->table[index].key, key) == 0) {
            // Supprimer l'entrée
            free(map->table[index].key); // Libérer la mémoire de la clé
            map->table[index].key = TOMBSTONE; ; // Marquer comme supprimé
            map->table[index].value = NULL; //Libérer la valeur
            return 1; // Suppression réussie
        }
        index = (index + 1) % map->size;
        if (index == index_original) {
            break; // Nous avons fait un tour complet
        }
    }
    return 0; // Clé non trouvée
}

void hashmap_destroy(HashMap *map) {
    if (map == NULL) {
        return;
    }
    if (map->table != NULL) {
        // Libérer chaque entrée de la table
        for (int i = 0; i < map->size; i++) {
            // Libérer la clé et la valeur si elles existent
            if (map->table[i].key != NULL && map->table[i].key != TOMBSTONE) {
                free(map->table[i].key);
                free(map->table[i].value);
            }
        }
        free(map->table);
    }
    free(map);
}
