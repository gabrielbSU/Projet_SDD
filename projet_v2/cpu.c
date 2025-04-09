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

    int* IP = (int*)malloc(sizeof(int));
    int* ZF = (int*)malloc(sizeof(int));
    int* SF = (int*)malloc(sizeof(int));

    int* SP = (int*)malloc(sizeof(int));
    int* BP = (int*)malloc(sizeof(int));

    if(AX == NULL || BX == NULL ||CX == NULL || DX == NULL || IP == NULL || ZF == NULL || SF == NULL || SP == NULL || BP == NULL){
        printf("Erreur d'allocation memoire\n");
        return NULL;
    }
    *AX = 0;
    *BX = 0;
    *CX = 0;
    *DX = 0;

    *IP = 0;
    *ZF = 0;
    *SF = 0;

    *ZF = 0;
    *SF = 0;

    hashmap_insert(cpu->context, "AX", AX);
    hashmap_insert(cpu->context, "BX", BX);
    hashmap_insert(cpu->context, "CX", CX);
    hashmap_insert(cpu->context, "DX", DX);

    hashmap_insert(cpu->context, "IP", IP);
    hashmap_insert(cpu->context, "ZF", ZF);
    hashmap_insert(cpu->context, "SF", SF);

    hashmap_insert(cpu->context, "SP", SP);
    hashmap_insert(cpu->context, "BP", BP);

    //if(create_segment(cpu->memory_handler,"SS",0,128)==0){ //todo
    //    printf("Echec de création_segment\n");
    //    return;
    //}

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

char *trim(char *str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;

    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }

    return str;
}

int search_and_replace(char **str, HashMap *values) {
    if (!str || !*str || !values) return 0;

    int replaced = 0;
    char *input = *str;

    // Iterate through all keys in the hashmap
    for (int i = 0; i < values->size; i++) {
        if (values->table[i].key && values->table[i].key != (void *)-1) {
            char *key = values->table[i].key;
            int value = *(int*)values->table[i].value;

            // Find potential substring match
            char *substr = strstr(input, key);

            if (substr) {
                // Construct replacement buffer
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                // Calculate lengths
                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                //int remain_len = strlen(substr + key_len);

                // Create new string
                char *new_str = (char *)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);

                new_str[substr - input] = '\0';

                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);
                // Free and update original string
                free(input);
                *str = new_str;
                input = new_str;

                replaced = 1;
            }
        }
    }

    // Trim the final string
    if (replaced) {
        char *trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced;
}

int resolve_constants(ParserResult *result){
    for(int i = 0; i< result->data_count; i++){
        if(result->code_instructions[i]->operand2 == NULL){
            void* val = hashmap_get(result->labels, result->code_instructions[i]->operand1);
            if(val==NULL){
                printf("Erreur, Label %s non trouvé\n", result->code_instructions[i]->operand1);
                return 0;
            }
            
            char str[20];
            sprintf(str, "%d", *(int*)val);
            free(result->code_instructions[i]->operand1);
            result->code_instructions[i]->operand1 = strdup(str);
        }else if(search_and_replace(&result->code_instructions[i]->operand2, result->memory_locations) == 0){
            printf("Erreur, operand %s non trouvé\n", result->code_instructions[i]->operand2);
            return 0;
        } 
    }
    return 1;
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count){
    Segment *ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");

    if(create_segment(cpu->memory_handler,"CS",ds->size,code_count)==0){
        printf("Echec de création_segment\n");
        return;
    }
    for(int i = 0; i<code_count; i++){
        //int *val = 
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }

    void* ip = hashmap_get(cpu->context, "IP");
    *(int*)ip = 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest){
    if(strcmp(instr->mnemonic, "MOV")==0){
        handle_MOV(cpu, src, dest);
        return 1;
    }else if(strcmp(instr->mnemonic, "ADD")==0){
        *(int*)dest += *(int*)src;
        return 1;
    }else if(strcmp(instr->mnemonic, "CMP")==0){
        int cmp = *(int*)dest-*(int*)src;
        if(cmp == 0){
            void* zf = hashmap_get(cpu->context, "ZF");
            *(int*)zf = 1;

            void* sf = hashmap_get(cpu->context, "SF");
            *(int*)sf = 0;
        }else if(cmp < 0){
            void* sf = hashmap_get(cpu->context, "SF");
            *(int*)sf = 1;

            void* zf = hashmap_get(cpu->context, "ZF");
            *(int*)zf = 0;
        }
        return 1;
    }else if(strcmp(instr->mnemonic, "JMP")==0){
        void* ip = hashmap_get(cpu->context, "IP");
        *(int*)ip = *(int*)resolve_adressing(cpu, instr->operand1);
        return 1;
    }else if(strcmp(instr->mnemonic, "JZ")==0){
        void* zf = hashmap_get(cpu->context, "ZF");
        if(*(int*)zf == 1){
            void* ip = hashmap_get(cpu->context, "IP");
            *(int*)ip = *(int*)resolve_adressing(cpu, instr->operand1);
        }
        return 1;
    }else if(strcmp(instr->mnemonic, "JNZ") == 0){
        void* zf = hashmap_get(cpu->context, "ZF");
        if(*(int*)zf == 0){
            void* ip = hashmap_get(cpu->context, "IP");
            *(int*)ip = *(int*)resolve_adressing(cpu, instr->operand1);
        }
        return 1;
    }else if(strcmp(instr->mnemonic, "HALT")==0){
        void* ip = hashmap_get(cpu->context, "IP");
        Segment *cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");

        *(int*)ip = cs->size;
        return 1; 
    }

    return 0;
}

int execute_instruction(CPU *cpu, Instruction *instr){
    void* src =  resolve_adressing(cpu, instr->operand2);
    void* dest =  resolve_adressing(cpu, instr->operand1);
    return handle_instruction(cpu, instr, src, dest);
}

Instruction *fetch_next_instruction(CPU *cpu){
    void* ip = hashmap_get(cpu->context, "IP");

    void* inst = load(cpu->memory_handler, "CS", *(int*)ip);    

    *(int*)ip +=1;

    return inst;
}

void print_registers(CPU *cpu){
    void *reg = hashmap_get(cpu->context, "AX");
    printf("AX -> %i\n", *(int*)reg);
    reg = hashmap_get(cpu->context, "BX");
    printf("BX -> %i\n", *(int*)reg);
    reg = hashmap_get(cpu->context, "CX");
    printf("CX -> %i\n", *(int*)reg);
    reg = hashmap_get(cpu->context, "DX");
    printf("DX -> %i\n", *(int*)reg);
    reg = hashmap_get(cpu->context, "IP");
    printf("IP -> %i\n", *(int*)reg);
    reg = hashmap_get(cpu->context, "ZF");
    printf("ZF -> %i\n", *(int*)reg);
    reg = hashmap_get(cpu->context, "SF");
    printf("SF -> %i\n", *(int*)reg);
}

int run_program(CPU *cpu){
    printf("--Etat initial--\n");
    print_data_segment(cpu);
    print_registers(cpu);

    printf("Execution...\n");
    Instruction * inst = fetch_next_instruction(cpu);
    execute_instruction(cpu, inst);

    printf("--Etat Final--\n");
    print_data_segment(cpu);
    print_registers(cpu);
}

int push_value(CPU *cpu, int value){
    void *sp = hashmap_get(cpu->context, "SP");
    if(sp == NULL){
        return -1;
    }
    if(*(int*)sp==128){
        //print stack overflow
        return -1;
    }
    *(int*)sp -=1;
}

int pop_value(CPU *cpu, int *dest){
    void *sp = hashmap_get(cpu->context, "SP");
    if(sp == NULL){
        return -1;
    }
    if(*(int*)sp==0){
        //print stack overflow
        return -1;
    }
    *(int*)sp +=1;
}