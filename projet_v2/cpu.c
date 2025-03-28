#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "cpu.h"

CPU *cpu_init(int memory_size){
    CPU *cpu = (CPU*)malloc(sizeof(CPU));
    
    if (cpu == NULL) { 
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }
    
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();
    cpu->constant_pool = hashmap_create();

    int* AX = (int*)malloc(sizeof(int));
    int* BX = (int*)malloc(sizeof(int));
    int* CX = (int*)malloc(sizeof(int));
    int* DX = (int*)malloc(sizeof(int));
    if(AX == NULL || BX == NULL ||CX == NULL || DX == NULL){
        printf("Erreur d'allocation memoire\n");
        return NULL;
    }
    *AX = 0;
    *BX = 0;
    *CX = 0;
    *DX = 0;

    hashmap_insert(cpu->context, "AX", AX);
    hashmap_insert(cpu->context, "BX", BX);
    hashmap_insert(cpu->context, "CX", CX);
    hashmap_insert(cpu->context, "DX", DX);
    return cpu;
}
void cpu_destroy(CPU *cpu){
    hashmap_destroy(cpu->context);
    hashmap_destroy(cpu->constant_pool);
    memoryHandler_destroy(cpu->memory_handler);
    free(cpu);
}
void *store(MemoryHandler *handler,const char *segment_name,int pos, void*data){
    Segment *seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if(seg==NULL){
        printf("Erreur, segment non trouvé\n");
        return NULL;
    }
    if(pos>seg->size){
        printf("Erreur, la position depasse la taille du segment\n");
        return NULL;
    }
    handler->memory[seg->start+pos] = data;

    // retourne data pour verifier si la fonction est executée avec succès, sinon retourne NULL.
    return data;
}
void *load(MemoryHandler *handler,const char *segment_name,int pos){
    Segment *seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    return handler->memory[seg->start+pos];
}
void allocate_variables(CPU *cpu, Instruction **data_instructions,int data_count){
    int total_size = 0;
    for(int i = 0; i<data_count; i++){
        Instruction *inst = data_instructions[i];
        int nb_comma = 0;
        for(int i = 0; i<strlen(inst->operand2); i++){
            if(inst->operand2[i] == ','){
                nb_comma++;
            }
        }
        total_size += nb_comma+1;
    }
    
    if(create_segment(cpu->memory_handler,"DS",0,total_size)==0){
        printf("Echec de création_segment\n");
        return;
    }

    int curr_offset = 0;
    for (int i = 0; i < data_count; i++) {
        Instruction *inst = data_instructions[i];
        char *data = strtok(inst->operand2, ", ");
        
        while (data != NULL) {
            int *new_data = (int *)malloc(sizeof(int));
            if (new_data == NULL) {
                printf("Erreur d'allocation memoire\n");
                return;
            }
            
            *new_data = atoi(data);
            if(store(cpu->memory_handler, "DS", curr_offset, new_data)==NULL){
                printf("Echec du stockage %s\n", data);
                return;
            }
            curr_offset++;
            
            data = strtok(NULL, ", ");
        }
    }
}
void print_data_segment(CPU *cpu){
    Segment *seg = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    for(int i = seg->start; i<seg->start+seg->size; i++){
        void *data = cpu->memory_handler->memory[i];
        if(data!=NULL){
            printf("DS %i: %i\n", i, *(int*)data);
        }
    }
}

int matches(const char *pattern, const char *string) {
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result) {
        fprintf(stderr, "Regex compilation failed for pattern: %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);  

    return result == 0; // Retourne 1 si la correspondance est trouvée, sinon 0

}

void *immediate_addressing(CPU *cpu, const char *operand){
    if(matches("^[A-Z]+ [A-Z]+,[0-9]+$", operand) == 0){
        return NULL;
    }

    char op[32];
    strcpy(op, operand);

    int* val = (int*)malloc(sizeof(int));
    char *token1;
    char *token2;
    token1 = strtok(op, " "); // mnemonic
    token1 = strtok(NULL, " "); // operand1,operand2
    token2 = strtok(token1, ","); // operand1
    token1 = strtok(NULL, ","); // operand2

    *val = atoi(token1);
    hashmap_insert(cpu->constant_pool, token2, val);
    return val;

}

void *register_addressing(CPU *cpu, const char *operand){
    if(matches("^[A-Z]+ [A-Z]+,[A-Z]+$", operand) == 0){
        return NULL;
    }

    char op[32];
    strcpy(op, operand);

    char *token1;
    token1 = strtok(op, " "); // mnemonic
    token1 = strtok(NULL, " "); // operand1,operand2
    token1 = strtok(token1, ","); // operand1
    token1 = strtok(NULL, ","); // operand2

    void* val = hashmap_get(cpu->context, token1);

    if(val == NULL){
        printf("Erreur, absence de valeur '%s' dans DS\n", token1);
        return NULL;
    }

    return val;
}

void *memory_direct_addressing(CPU *cpu, const char *operand){
    if(matches("^[A-Z]+ [A-Z]+,\\[[0-9]+\\]$", operand) == 0){
        return NULL;
    }

    char op[32];
    strcpy(op, operand);
    
    char *token1;

    token1 = strtok(op, " "); // mnemonic
    token1 = strtok(NULL, " "); // operand1,[operand2]
    token1 = strtok(token1, ","); // operand1
    token1 = strtok(NULL, ","); // [operand2]
    token1 = strtok(token1, "["); // operand2]
    token1 = strtok(token1, "]"); // operand2

    void* val = load(cpu->memory_handler, "DS", atoi(token1));
    
    if(val==NULL){
        printf("Erreur, absence de valeur position '%s' dans DS\n", token1);
        return NULL;
    }
    
    return val;
}

void *register_indirect_addressing(CPU *cpu, const char *operand){
    if(matches("^[A-Z]+ [A-Z]+,\\[[A-Z]+\\]$", operand) == 0){
        return NULL;
    }

    char op[32];
    strcpy(op, operand);

    char *token1;
    token1 = strtok(op, " "); // mnemonic
    token1 = strtok(NULL, " "); // operand1,[operand2]
    token1 = strtok(token1, ","); // operand1
    token1 = strtok(NULL, ","); // [operand2]
    token1 = strtok(token1, "["); // operand2]
    token1 = strtok(token1, "]"); // operand2
    
    void* i = hashmap_get(cpu->context, token1);

    if(i == NULL){
        printf("Erreur, absence de valeur '%s' dans cpu->context\n", token1);
        return NULL;
    }

    void* val = load(cpu->memory_handler, "DS", *(int*)i);
    
    if(val==NULL){
        printf("Erreur, absence de valeur position '%i' dans cpu->memory_handler\n", *(int*)i);
        return NULL;
    }

    return val;
}

void handle_MOV(CPU *cpu, void *src, void *dest){
    memcpy(dest, src, sizeof(int));
}

void *resolve_adressing(CPU *cpu,const char *operand){
    void *data = immediate_addressing(cpu, operand);
    if(data != NULL){
        return data;
    }
    data = register_addressing(cpu, operand);
    if(data != NULL){
        return data;
    }
    data = memory_direct_addressing(cpu, operand);
    if(data != NULL){
        return data;
    }
    data = register_indirect_addressing(cpu, operand);
    if(data != NULL){
        return data;
    }

    return NULL;
}



