#include <stdlib.h>
#include <string.h>
#include "dynamique.h"
#include "parser.h"
#include "logger.h"

MemoryHandler *memory_init(int size) {
    // Initialisation du gestionnaire de mémoire
    MemoryHandler *handler = (MemoryHandler *)malloc(sizeof(MemoryHandler));
    if (handler == NULL) {
        LOG_ERROR("échec d'allocation mémoire pour le gestionnaire");
        return NULL;
    }
    // Initialisation de la mémoire du gestionnaire
    handler->memory = (void **)malloc(size * sizeof(void *));
    if (handler->memory == NULL) {
        LOG_ERROR("échec d'allocation mémoire pour le tableau de mémoire");
        free(handler);
        return NULL;
    }
    // Initialisation de la mémoire à NULL
    for (int i = 0; i < size; i++) {
        handler->memory[i] = NULL;
    }
    handler->total_size = size; // Initialisation de la taille totale de la mémoire


    // Initialisation de la liste des segments libres
    Segment *free_segment = (Segment *)malloc(sizeof(Segment));
    if (free_segment == NULL) {
        LOG_ERROR("échec d'allocation mémoire pour le segment libre");
        free(handler->memory);
        free(handler);
        return NULL;
    }
    free_segment->start = 0;
    free_segment->size = size;
    free_segment->next = NULL;
    handler->free_list = free_segment;

    // Initialisation de la table de hachage pour les segments alloués
    handler->allocated = hashmap_create();
    if (handler->allocated == NULL) {
        LOG_ERROR("échec d'initialisation de la table de hachage");
        free(free_segment);
        free(handler->memory);
        free(handler);
        return NULL;
    }

    return handler; // Retourne le gestionnaire de mémoire initialisé
}

Segment *find_free_segment(MemoryHandler* handler,int start,int size,Segment** prev){
    Segment *current = handler->free_list;
    *prev=NULL; // Initialiser le pointeur précédent à NULL
    while (current !=NULL){
        // Vérifier si le segment libre est suffisamment grand et s'il est dans la plage demandée
        if (current->start<=start && (current->start + current->size)>= (start + size)){
            return current;
        }
        *prev = current; // Mettre à jour le pointeur précédent
        current =current ->next; // Passer au segment suivant
    }

    return NULL; // Aucun segment libre trouvé
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size) {
    // Vérifier les paramètres
    if (handler == NULL || name == NULL || size <= 0 || start < 0) {
        LOG_ERROR("paramètres invalides pour la création du segment");
        return 0;
    }

    // Vérifier si l'espace mémoire demandé est disponible
    Segment *prev;
    Segment *free_segment = find_free_segment(handler, start, size, &prev);
    if (free_segment == NULL) {
        LOG_ERROR("aucun segment libre trouvé pour l'allocation");
        return 0;
    }

    // Créer un nouveau segment alloué
    Segment *new_seg = (Segment *)malloc(sizeof(Segment));
    if (new_seg == NULL) {
        LOG_ERROR("échec d'allocation mémoire pour le nouveau segment");
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
            LOG_ERROR("Erreur d'allocation mémoire pour le segment libre avant");
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
            LOG_ERROR("Erreur d'allocation mémoire pour le segment libre après");
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
        LOG_ERROR("Erreur : paramètres invalides");
        return 0;
    }

    // Rechercher le segment alloué dans la table de hachage
    Segment *seg = (Segment *)hashmap_get(handler->allocated, name);
    if (seg == NULL) {
        LOG_ERROR("Erreur : segment alloué non trouvé");
        return 0;
    }

    // Retirer le segment de la table de hachage
    if (hashmap_remove(handler->allocated, name) != 1) {
        LOG_ERROR("Erreur : impossible de retirer le segment de la table de hachage");
        return 0;
    }

    // Ajouter le segment libéré à la liste des segments libres
    Segment *new_free = (Segment *)malloc(sizeof(Segment));
    if (new_free == NULL) {
        LOG_ERROR("Erreur d'allocation mémoire pour le segment libre");
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

void memoryHandler_destroy(MemoryHandler *handler){
    // Libération de la mémoire allouée pour les segments de données
    Segment *DS = (Segment*)hashmap_get(handler->allocated, "DS");
    for(int i = 0; i<DS->size; i++){
        free(handler->memory[DS->start+i]);
    }

    // Libération de la mémoire allouée pour les segments de données
    Segment *CS = (Segment*)hashmap_get(handler->allocated, "CS");
    for(int i = 0; i<CS->size; i++){
        Instruction *inst = (Instruction *)handler->memory[CS->start+i];
        if(inst == NULL){
            continue;
        }        
        free(inst->mnemonic);
        free(inst->operand1);

        free(inst->operand2);
        free(inst);
    }

    // Libération de la mémoire allouée pour le gestionnaire de mémoire
    hashmap_destroy(handler->allocated);
    Segment *seg = handler->free_list;
    while (seg != NULL) {
        Segment *tmp = seg->next;
        free(seg);
        seg = tmp;
    }

    free(handler->memory); // Libération de la mémoire allouée pour le tableau de mémoire
    free(handler); // Libération du gestionnaire de mémoire
}