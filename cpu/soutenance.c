/* Tenter de l'integrer a votre projet. 
Le faire compiler et l'executer sans erreur ni fuite 
de memoire ;) */

#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{

    MemoryHandler* MH = memory_init(17); //Q2.1
    create_segment(MH, "MS", 0, 4); //Q2.3
    create_segment(MH, "AS", 5, 2); //Q2.3
    create_segment(MH, "DS", 8, 6); //Q2.3
    create_segment(MH, "CS", 14, 2); //Q2.3

    /* Affichage de la liste chainee des segments libres*/
    Segment *tmp = MH->free_list;
    while(tmp != NULL){
        printf("[%d, %d] ", tmp->start, tmp->size);
        tmp = tmp->next;
        if(tmp != NULL)
            printf("-> ");
    }
    printf("\n");
    /* Fin affichage */

    tmp = hashmap_get(MH->allocated, "MS"); //Q1.4
    printf("\n[%d %d] - ", tmp->start, tmp->size);
    tmp = hashmap_get(MH->allocated, "AS"); //Q1.4
    printf("[%d %d] - ", tmp->start, tmp->size);
    tmp = hashmap_get(MH->allocated, "DS"); //Q1.4
    printf("[%d %d] - ", tmp->start, tmp->size);
    tmp = hashmap_get(MH->allocated, "CS"); //Q1.4
    printf("[%d %d]\n\n", tmp->start, tmp->size);

    remove_segment(MH, "MS");
    remove_segment(MH, "AS");
    
    /* Affichage de la liste chainee des segments libres*/
    tmp = MH->free_list;
    while(tmp != NULL){
        printf("[%d, %d] ", tmp->start, tmp->size);
        tmp = tmp->next;
        if(tmp != NULL)
            printf("-> ");
    }
    printf("\n");
    /* Fin affichage */

    //memoryhandler_destroy(MH); // (non demande dans le projet) libere la memoire occupee par
                              // la structure memoryhandler 

    return 0;
}

