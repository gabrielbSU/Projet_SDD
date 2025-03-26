#include "parser.h"
#include "cpu.h"

CPU* setup_test_environment() {
    // Initialiser le CPU
    CPU* cpu = cpu_init(1024);
    if (!cpu) {
        printf("Error: CPU initialization failed\n");
        return NULL;
    }

    // Initialiser les registres avec des valeurs spécifiques
    int* ax = (int*)hashmap_get(cpu->context, "AX");
    int* bx = (int*)hashmap_get(cpu->context, "BX");
    int* cx = (int*)hashmap_get(cpu->context, "CX");
    int* dx = (int*)hashmap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;

    // Créer et initialiser le segment de données
    if (!hashmap_get(cpu->memory_handler->allocated, "DS")) {
        create_segment(cpu->memory_handler, "DS", 0, 20);

        // Initialiser le segment de données avec des valeurs de test
        for (int i = 0; i < 10; i++) {
            int* value = (int*)malloc(sizeof(int));
            *value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
            store(cpu->memory_handler, "DS", i, value);
        }
    }
    printf("Test environment initialized.\n");
    return cpu;
}

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