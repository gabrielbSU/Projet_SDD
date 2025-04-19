#ifndef CPU_H
#define CPU_H

#include "hachage.h"
#include "dynamique.h"
#include "parser.h"

// Structure CPU
typedef struct {
    MemoryHandler* memory_handler; //Gestionnaire de memoire
    HashMap *context; //Registres
    HashMap *constant_pool; //Table de hachage pour stocker les valeurs immediates
} CPU;

// Fonctions de gestion du CPU
CPU *cpu_init(int memory_size);
void cpu_destroy(CPU *cpu);
void *store(MemoryHandler *handler,const char *segment_name,int pos, void*data);
void *load(MemoryHandler *handler,const char *segment_name,int pos);
void allocate_variables(CPU *cpu,Instruction **data_instructions,int data_count);
int alloc_es_segment(CPU *cpu);
void print_data_segment(CPU *cpu);
void *immediate_addressing(CPU *cpu, const char *operand);
void *register_addressing(CPU *cpu, const char *operand);
void *memory_direct_addressing(CPU *cpu, const char *operand);
void *register_indirect_addressing(CPU *cpu, const char *operand);
void *segment_override_addressing(CPU* cpu, const char* operand);
void *resolve_adressing(CPU *cpu,const char *operand);
int resolve_constants(ParserResult *result);
void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count);
int run_program(CPU *cpu);
int push_value(CPU *cpu, int value);
int pop_value(CPU *cpu, int *dest);
#endif