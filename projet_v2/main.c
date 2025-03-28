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
    CPU *cpu = setup_test_environment();
    print_data_segment(cpu);
    
    void * tok = resolve_adressing(cpu, "MOV AX,[CX]");
    tok = resolve_adressing(cpu, "MOV AX,[AX]");
    printf("MOV AX,[AX]: %i\n", *(int*)tok);
    tok = resolve_adressing(cpu, "MOV AX,[8]");
    printf("MOV AX,[8]: %i\n", *(int*)tok);
    tok = resolve_adressing(cpu, "MOV AX,CX");
    printf("MOV AX,CX: %i\n", *(int*)tok);
    tok = resolve_adressing(cpu, "MOV AX,42");
    printf("MOV AX,42: %i\n", *(int*)tok);

    void *dest = hashmap_get(cpu->context, "AX");
    handle_MOV(cpu, tok, dest);
    printf("test MOV: %i %i\n", *(int*)tok, *(int*)resolve_adressing(cpu, "MOV AX,AX"));

    cpu_destroy(cpu);
    
    return 0;
}