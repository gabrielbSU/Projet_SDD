#include <string.h>
#include "dynamique.h"
#include "parser.h"
#include "logger.h"

MemoryHandler *memory_init(int size) {
    // Initialisation du gestionnaire de memoire
    MemoryHandler *handler = (MemoryHandler *)malloc(sizeof(MemoryHandler));
    LOG_ASSERT(handler!=NULL, "echec d'allocation memoire pour le gestionnaire");

    // Initialisation de la memoire du gestionnaire
    handler->memory = (void **)malloc(size * sizeof(void *));
    LOG_ASSERT(handler->memory!=NULL, "echec d'allocation memoire pour le tableau de memoire");

    // Initialisation de la memoire à NULL
    for (int i = 0; i < size; i++) {
        handler->memory[i] = NULL;
    }
    handler->total_size = size; // Initialisation de la taille totale de la memoire


    // Initialisation de la liste des segments libres
    Segment *free_segment = (Segment *)malloc(sizeof(Segment));
    LOG_ASSERT(free_segment!=NULL, "echec d'allocation memoire pour le segment libre");

    free_segment->start = 0;
    free_segment->size = size;
    free_segment->next = NULL;
    handler->free_list = free_segment;

    // Initialisation de la table de hachage pour les segments alloues
    handler->allocated = hashmap_create();
    LOG_ASSERT(handler->allocated!=NULL, "echec d'initialisation de la table de hachage");

    return handler; // Retourne le gestionnaire de memoire initialise
}

Segment *find_free_segment(MemoryHandler* handler,int start,int size,Segment** prev){
    Segment *current = handler->free_list;
    *prev=NULL; // Initialiser le pointeur precedent à NULL
    while (current !=NULL){
        // Verifier si le segment libre est suffisamment grand et s'il est dans la plage demandee
        if (current->start<=start && (current->start + current->size)>= (start + size)){
            return current;
        }
        *prev = current; // Mettre à jour le pointeur precedent
        current =current ->next; // Passer au segment suivant
    }

    return NULL; // Aucun segment libre trouve
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
    // Verifier les paramètres
    if (handler == NULL || name == NULL || size <= 0 || start < 0) {
        LOG_ERROR("paramètres invalides pour la creation du segment");
        return 0;
    }

    // Verifier si l'espace memoire demande est disponible
    Segment *prev;
    Segment *free_segment = find_free_segment(handler, start, size, &prev);
    if (free_segment == NULL) {
        LOG_ERROR("aucun segment libre trouve pour l'allocation");
        return 0;
    }

    // Creer un nouveau segment alloue
    Segment *new_seg = (Segment *)malloc(sizeof(Segment));
    if (new_seg == NULL) {
        LOG_ERROR("echec d'allocation memoire pour le nouveau segment");
        return 0;
    }
    new_seg->start = start;
    new_seg->size = size;
    new_seg->next = NULL;

    if (hashmap_insert(handler->allocated, name, new_seg) != 1) {
        LOG_ERROR("impossible d'ajouter le segment à la table de hachage");
        free(new_seg);
        return 0;
    }
    
    // Cas 1 : Le segment libre commence avant `start`
    if (free_segment->start < start) {
        Segment *before = (Segment *)malloc(sizeof(Segment));
        if (before == NULL) {
            LOG_ERROR("echec d'allocation memoire du segment libre");
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
            LOG_ERROR("Erreur d'allocation memoire pour le segment libre après");
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

    free(free_segment); // Liberer la memoire on cree pour l'insertion 

    return 1;
}

int remove_segment(MemoryHandler *handler, const char *name) {
    // Verifier les paramètres
    if (handler == NULL || name == NULL) {
        LOG_ERROR("Erreur : paramètres invalides");
        return 0;
    }

    // Rechercher le segment alloue dans la table de hachage
    Segment *seg = (Segment *)hashmap_get(handler->allocated, name);
    if (seg == NULL) {
        LOG_ERROR("Erreur : segment alloue non trouve");
        return 0;
    }

    // Retirer le segment de la table de hachage
    if (hashmap_remove(handler->allocated, name) != 1) {
        LOG_ERROR("Erreur : impossible de retirer le segment de la table de hachage");
        return 0;
    }

    // Ajouter le segment libere à la liste des segments libres
    Segment *new_free = (Segment *)malloc(sizeof(Segment));
    if (new_free == NULL) {
        LOG_ERROR("Erreur d'allocation memoire pour le segment libre");
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
        // Fusionner avec le segment libre precedent
        else if (current->start + current->size == new_free->start) {
            current->size += new_free->size;
            free(new_free);
            new_free = current;
        }
        // Inserer le segment libre dans la liste
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

    // Liberer le segment alloue
    free(seg);

    return 1;
}

int first_fit_strategy(MemoryHandler *handler, int size){
    Segment *current = handler->free_list;
    while (current !=NULL){
        if (size <= current->size ){ // si le segment libre est suffisamment grand
            return current->start;  // Renvoie sa position si le segment libre est suffisamment grand
        }
        current =current ->next; // Passer au segment suivant
    }

    return -1; // Aucun segment libre trouve
}

int best_fit_strategy(MemoryHandler *handler, int size){
    Segment *current = handler->free_list;
    int size_min = 0x7FFFFFFF;  // valeur maximale possible
    int start = -1; // position du segment
    while (current !=NULL){
        if (size <= current->size && current->size<size_min){ // si le segment libre est suffisamment grand
            start = current->start; // Met a jour sa position
            size_min = current->size;   // Mis a jour de la taille minimale
        }
        current =current ->next; // Passer au segment suivant
    }

    return start; // Renvoie la position du segment
}

int worst_fit_strategy(MemoryHandler *handler, int size){
    Segment *current = handler->free_list;
    int size_max = 0;  // valeur minimal possible
    int start = -1; // position du segment
    while (current !=NULL){
        if (size <= current->size && current->size>size_max){ // si le segment libre est suffisamment grand
            start = current->start; // Met a jour sa position
            size_max = current->size;   // Mis a jour de la taille maximale
        }
        current = current ->next; // Passer au segment suivant
    }

    return start; // Renvoie la position du segment
}

int find_free_address_strategy(MemoryHandler *handler, int size, int strategy){
    // selectionne la strategie
    switch (strategy) {
    case 0:
        return first_fit_strategy(handler, size);
        break;
    case 1:
        return best_fit_strategy(handler, size);
        break;
    case 2:
        return worst_fit_strategy(handler, size);
        break;
    default:
        LOG_ERROR("strategie d'adressage \"%i\" invalide", strategy);
        return -1;
        break;
    }
}