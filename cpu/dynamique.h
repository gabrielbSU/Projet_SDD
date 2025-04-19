#ifndef DYNAMIQUE_H
#define DYNAMIQUE_H

#include "hachage.h"

// Structure pour représenter un segment de mémoire
typedef struct segment {
    int start; // Position de début (adresse) du segment dans la mémoire
    int size;  // Taille du segment en unités de mémoire
    struct segment *next; // Pointeur vers le segment suivant dans la liste chaînée
} Segment;

// Structure pour gérer la mémoire
typedef struct memoryHandler {
    void **memory; // Tableau de pointeurs vers la mémoire allouée
    int total_size; // Taille totale de la mémoire gérée
    Segment *free_list; // Liste chaînée des segments de mémoire libres
    HashMap *allocated; // Table de hachage (nom, segment)
} MemoryHandler;

// Fonctions de gestion de la mémoire
MemoryHandler *memory_init(int size);
Segment *find_free_segment(MemoryHandler* handler,int start,int size,Segment** prev);
int create_segment(MemoryHandler *handler,const char *name,int start,int size);
int remove_segment(MemoryHandler *handler,const char*name);
int find_free_address_strategy(MemoryHandler *handler, int size, int strategy);

#endif
