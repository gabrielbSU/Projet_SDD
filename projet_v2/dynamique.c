#include "dynamique.h"

MemoryHandler *memory_init(int size) {

    MemoryHandler *handler = (MemoryHandler *)malloc(sizeof(MemoryHandler));
    if (handler == NULL) {
        printf("Erreur d'allocation mémoire pour le gestionnaire\n");
        return NULL;
    }

    handler->memory = (void **)malloc(size * sizeof(void *));
    if (handler->memory == NULL) {
        printf("Erreur d'allocation mémoire pour le tableau de mémoire\n");
        free(handler);
        return NULL;
    }

    handler->total_size = size;

    Segment *free_segment = (Segment *)malloc(sizeof(Segment));
    if (free_segment == NULL) {
        printf("Erreur d'allocation mémoire pour le segment libre\n");
        free(handler->memory);
        free(handler);
        return NULL;
    }
    free_segment->start = 0;
    free_segment->size = size;
    free_segment->next = NULL;
    handler->free_list = free_segment;

    handler->allocated = hashmap_create();
    if (handler->allocated == NULL) {
        printf("Erreur d'initialisation de la table de hachage\n");
        free(free_segment);
        free(handler->memory);
        free(handler);
        return NULL;
    }

    return handler;
}

Segment *find_free_segment(MemoryHandler* handler,int start,int size,Segment** prev){
    Segment *current = handler->free_list;
    *prev=NULL; // Initialiser le pointeur précédent à NULL

    while (current !=NULL){
        if (current->start<=start && (current->start + current->size)>= (start + size)){
            return current;
        }
        *prev = current;
        current =current ->next;
    }

    return NULL;
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
    // Vérifier les paramètres
    if (handler == NULL || name == NULL || size <= 0 || start < 0) {
        printf("Erreur : paramètres invalides\n");
        return 0;
    }

    // Vérifier si l'espace mémoire demandé est disponible
    Segment *prev;
    Segment *free_segment = find_free_segment(handler, start, size, &prev);
    if (free_segment == NULL) {
        printf("Erreur : aucun segment libre trouvé pour l'allocation\n");
        return 0;
    }

    // Créer un nouveau segment alloué
    Segment *new_seg = (Segment *)malloc(sizeof(Segment));
    if (new_seg == NULL) {
        printf("Erreur d'allocation mémoire pour le nouveau segment\n");
        return 0;
    }
    new_seg->start = start;
    new_seg->size = size;
    new_seg->next = NULL;

    if (hashmap_insert(handler->allocated, name, new_seg) != 1) {
        printf("Erreur : impossible d'ajouter le segment à la table de hachage\n");
        free(new_seg);
        return 0;
    }
    
    // Cas 1 : Le segment libre commence avant `start`
    if (free_segment->start < start) {
        Segment *before = (Segment *)malloc(sizeof(Segment));
        if (before == NULL) {
            printf("Erreur d'allocation mémoire pour le segment libre avant\n");
            return 0;
        }
        before->start = free_segment->start;
        before->size = start - free_segment->start;
        before->next = free_segment->next;

        // Ajouter `before` à la liste des segments libres
        if (prev == NULL) {
            handler->free_list = before;
        } else {
            prev->next = before;
        }
    }

    // Cas 2 : Le segment libre se termine après `start + size`
    if (free_segment->start + free_segment->size > start + size) {
        Segment *after = (Segment *)malloc(sizeof(Segment));
        if (after == NULL) {
            printf("Erreur d'allocation mémoire pour le segment libre après\n");
            return 0;
        }
        after->start = start + size;
        after->size = free_segment->start + free_segment->size - (start + size);
        after->next = free_segment->next;

        // Ajouter `after` à la liste des segments libres
        if (prev == NULL) {
            handler->free_list = after;
        } else {
            prev->next = after;
        }
    }

    free(free_segment); // Libérer la mémoire on crée pour l'insertion 

    return 1;
}

int remove_segment(MemoryHandler *handler, const char *name) {
    // Vérifier les paramètres
    if (handler == NULL || name == NULL) {
        printf("Erreur : paramètres invalides\n");
        return 0;
    }

    // Rechercher le segment alloué dans la table de hachage
    Segment *seg = (Segment *)hashmap_get(handler->allocated, name);
    if (seg == NULL) {
        printf("Erreur : segment alloué non trouvé\n");
        return 0;
    }

    // Retirer le segment de la table de hachage
    if (hashmap_remove(handler->allocated, name) != 1) {
        printf("Erreur : impossible de retirer le segment de la table de hachage\n");
        return 0;
    }

    // Ajouter le segment libéré à la liste des segments libres
    Segment *new_free = (Segment *)malloc(sizeof(Segment));
    if (new_free == NULL) {
        printf("Erreur d'allocation mémoire pour le segment libre\n");
        return 0;
    }
    new_free->start = seg->start;
    new_free->size = seg->size;
    new_free->next = NULL;

    // Fusionner avec les segments libres adjacents
    Segment *current = handler->free_list;
    Segment *prev = NULL;
    while (current != NULL) {
        // Fusionner avec le segment libre suivant
        if (new_free->start + new_free->size == current->start) {
            new_free->size += current->size;
            new_free->next = current->next;
            free(current);
            if (prev == NULL) {
                handler->free_list = new_free;
            } else {
                prev->next = new_free;
            }
            current = new_free;
        }
        // Fusionner avec le segment libre précédent
        else if (current->start + current->size == new_free->start) {
            current->size += new_free->size;
            free(new_free);
            new_free = current;
        }
        // Insérer le segment libre dans la liste
        else if (new_free->start < current->start) {
            new_free->next = current;
            if (prev == NULL) {
                handler->free_list = new_free;
            } else {
                prev->next = new_free;
            }
            break;
        }
        prev = current;
        current = current->next;
    }

   // Si la liste des segments libres est vide, ajouter le nouveau segment libre
    if (handler->free_list == NULL) {
        handler->free_list = new_free;
    }

    // Libérer le segment alloué
    free(seg);

    return 1;
}