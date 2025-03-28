#ifndef CPU_H
#define CPU_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hachage.h"
#include "dynamique.h"
#include "parser.h"



typedef struct {
    MemoryHandler* memory_handler; //Gestionnaire de memoire
    HashMap *context; //Registres (AX, BX, CX, DX)
    HashMap *constant_pool; //Table de hachage pour stocker les valeurs immediates
} CPU;

CPU *cpu_init(int memory_size);
void cpu_destroy(CPU *cpu);
void *store(MemoryHandler *handler,const char *segment_name,int pos, void*data);
void *load(MemoryHandler *handler,const char *segment_name,int pos);
void allocate_variables(CPU *cpu,Instruction **data_instructions,int data_count);
void print_data_segment(CPU *cpu);
void *immediate_addressing(CPU *cpu, const char *operand);
void *register_addressing(CPU *cpu, const char *operand);
void *memory_direct_addressing(CPU *cpu, const char *operand);
void *register_indirect_addressing(CPU *cpu, const char *operand);
void handle_MOV(CPU *cpu, void *src, void *dest);
void *resolve_adressing(CPU *cpu,const char *operand);
#endif