#include "parser.h"
#include "cpu.h"

int main(){
    ParserResult *res = parse("code.asm");

    printf("start -> %i\n", *(int*)hashmap_get(res->labels, "start"));
    printf("loop -> %i\n",*(int*)hashmap_get(res->labels, "loop"));

    printf("x -> %i\n",  *(int*)hashmap_get(res->memory_locations, "x"));
    printf("arr -> %i\n",*(int*)hashmap_get(res->memory_locations, "arr"));
    printf("y -> %i\n",  *(int*)hashmap_get(res->memory_locations, "y"));

    CPU *cpu = cpu_init(256);
    allocate_variables(cpu, res->data_instructions, res->data_count);
    print_data_segment(cpu);

    cpu_destroy(cpu);
    free_parser_result(res);
    
    return 0;
}