#include <string.h>
#include <ctype.h>
#include "cpu.h"


CPU *cpu_init(int memory_size){
    return ;
}
void cpu_destroy(CPU *cpu){
    return ;
}
void *store(MemoryHandler *handler,const char *segment_name,int pos, void*data){
    return ;
}
void *load(MemoryHandler *handler,const char *segment_name,int pos){
    return ;
}
void allocate_variables(CPU *cpu,Instruction **data_instructions,int data_count){
    return ;
}
void print_data_segment(CPU *cpu){
    return ;
}