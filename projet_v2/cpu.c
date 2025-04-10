#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include <stdlib.h>
#include "cpu.h"
#include "logger.h"

CPU *cpu_init(int memory_size){
    // Initialisation mémoire cpu
    CPU *cpu = (CPU*)malloc(sizeof(CPU));
    
    if (cpu == NULL) { 
        LOG_ERROR("échec d'allocation mémoire\n");
        return NULL;
    }
    
    cpu->memory_handler = memory_init(memory_size);
    cpu->context = hashmap_create();
    cpu->constant_pool = hashmap_create();

    // Initialisation mémoire des registres
    int* AX = (int*)malloc(sizeof(int));    // AX
    int* BX = (int*)malloc(sizeof(int));    // BX
    int* CX = (int*)malloc(sizeof(int));    // CX
    int* DX = (int*)malloc(sizeof(int));    // DX

    int* IP = (int*)malloc(sizeof(int));    // Instruction Pointer
    int* ZF = (int*)malloc(sizeof(int));    // Zero Flag
    int* SF = (int*)malloc(sizeof(int));    // Sign Flag

    int* SP = (int*)malloc(sizeof(int));    // Stack Pointer
    int* BP = (int*)malloc(sizeof(int));    // Base Pointer

    if(AX == NULL || BX == NULL ||CX == NULL || DX == NULL || IP == NULL || ZF == NULL || SF == NULL || SP == NULL || BP == NULL){
        LOG_ERROR("échec d'allocation memoire\n");
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

    // Stockages des registres dans le contexte du cpu
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
    hashmap_destroy(cpu->context);  // libération des registres
    hashmap_destroy(cpu->constant_pool); // libération des valeurs imédiates
    memoryHandler_destroy(cpu->memory_handler); // libération du gestionnaire de memoire
    free(cpu); // libération du cpu
}
void *store(MemoryHandler *handler,const char *segment_name,int pos, void*data){
    // Récupération du segment de donné <segment_name>
    Segment *seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if(seg==NULL){
        LOG_ERROR("segment non trouvé\n");
        return NULL;
    }
    if(pos>seg->size){
        LOG_ERROR("Erreur, la position depasse la taille du segment\n");
        return NULL;
    }
    // stockage de la donnée dans le segment à sa position <pos>
    handler->memory[seg->start+pos] = data;

    // retourne data pour verifier si la fonction est executée avec succès, sinon retourne NULL.
    return data;
}
void *load(MemoryHandler *handler,const char *segment_name,int pos){
    // Récupération du segment de donné <segment_name>
    Segment *seg = (Segment*)hashmap_get(handler->allocated, segment_name);
    if(seg==NULL){
        LOG_ERROR("segment %s position %i non trouvé",segment_name, pos);
        return NULL;
    }
    // Retourne la donnée à la position <pos> du segment
    return handler->memory[seg->start+pos];
}
void allocate_variables(CPU *cpu, Instruction **data_instructions,int data_count){
    int total_size = 0;
    // Calcul de la taille totale requise pour le segment de données
    for(int i = 0; i<data_count; i++){
        Instruction *inst = data_instructions[i];
        int nb_comma = 0;
        // On compte le nombre de virgules dans l'opérande 2
        for(int i = 0; i<strlen(inst->operand2); i++){
            if(inst->operand2[i] == ','){
                nb_comma++;
            }
        }
        // On ajoute le nombre de virgules + 1 à la taille totale
        total_size += nb_comma+1;
    }
    
    // Création du segment de données (Data Segment)
    if(create_segment(cpu->memory_handler,"DS",0,total_size)==0){
        LOG_ERROR("Echec de création de segment \"DS\"\n");
        return;
    }

    // Stockage des données dans le segment de données
    int curr_offset = 0;
    for (int i = 0; i < data_count; i++) {
        // On récupère l'instruction
        Instruction *inst = data_instructions[i];
        
        // On récupère la donnée à stocker
        char *data = strtok(inst->operand2, ", ");
        
        while (data != NULL) {
            // Copier la donnée dans new_data
            int *new_data = (int *)malloc(sizeof(int));
            if (new_data == NULL) {
                LOG_ERROR("Echec d'allocation memoire\n");
                return;
            }
            
            *new_data = atoi(data);
            // On stocke la donnée dans le segment de données
            if(store(cpu->memory_handler, "DS", curr_offset, new_data)==NULL){
                LOG_ERROR("Echec du stockage %s\n", data);
                return;
            }
            // On met à jour l'offset courant
            curr_offset++;
            
            data = strtok(NULL, ", ");
        }
    }
}
void print_data_segment(CPU *cpu){
    // Récupération du segment de données
    Segment *seg = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    for(int i = seg->start; i<seg->start+seg->size; i++){
        // On récupère la donnée à la position i
        void *data = cpu->memory_handler->memory[i];
        if(data!=NULL){
            // On affiche la donnée
            printf("DS %i: %i\n", i, *(int*)data);
        }
    }
}

int matches(const char *pattern, const char *string) {
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    if (result) {
        LOG_ERROR("Regex compilation failed for pattern: %s", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);  

    return result == 0; // Retourne 1 si la correspondance est trouvée, sinon 0

}

void *immediate_addressing(CPU *cpu, const char *operand){
    // Vérification si l'opérande est un nombre entier
    // Exemple : 123, 456, 789, etc.
    if(matches("^[0-9]+$", operand) == 0){
        return NULL;
    }

    // Copie de la valeur de l'opérande dans val
    int* val = (int*)malloc(sizeof(int));
    *val = atoi(operand);

    // Insertion de la valeur dans le pool de constantes
    hashmap_insert(cpu->constant_pool, operand, val);
    return val; // retourne le pointeur vers la valeur
}

void *register_addressing(CPU *cpu, const char *operand){
    // Vérification si l'opérande est un registre
    // Exemple : AX, BX, CX, DX
    if(matches("^[A-Z]+$", operand) == 0){
        return NULL;
    }

    // Récupération de la valeur du registre dans le contexte du CPU
    void* val = hashmap_get(cpu->context, operand);

    if(val == NULL){
        LOG_ERROR("absence de valeur \"%s\" dans DS", operand);
        return NULL;
    }

    return val; // retourne le pointeur vers la valeur
}

void *memory_direct_addressing(CPU *cpu, const char *operand){
    // Vérification si l'opérande est une adresse mémoire directe
    // Exemple : [0], [1], [2], etc.
    if(matches("^\\[[0-9]+\\]$", operand) == 0){
        return NULL;
    }

    char op[32];
    strcpy(op, operand);
    
    char *token1;

    // On extrait la valeur entre crochets
    token1 = strtok(op, "["); // "operand2]"
    token1 = strtok(token1, "]"); // "operand2"

    // Récupération de la valeur dans le segment de données à la position spécifiée
    void* val = load(cpu->memory_handler, "DS", atoi(token1));
    
    if(val==NULL){
        LOG_ERROR("absence de valeur position \"%s\" dans DS", token1);
        return NULL;
    }
    
    return val; // retourne le pointeur vers la valeur
}

void *register_indirect_addressing(CPU *cpu, const char *operand){
    // Vérification si l'opérande est un registre indirect
    // Exemple : [AX], [BX], [CX], [DX]
    if(matches("^\\[[A-Z]+\\]$", operand) == 0){
        return NULL;
    }

    char op[32];
    strcpy(op, operand);

    char *token1;

    // On extrait la valeur entre crochets
    token1 = strtok(op, "["); // "operand2]"
    token1 = strtok(token1, "]"); // "operand2"
    
    // Récupération de la valeur du registre indirect
    void* i = hashmap_get(cpu->context, token1);

    if(i == NULL){
        LOG_ERROR("absence de valeur \"%s\" dans cpu->context", token1);
        return NULL;
    }

    // Récupération de la valeur dans le segment de données à la position spécifiée
    void* val = load(cpu->memory_handler, "DS", *(int*)i);
    
    if(val==NULL){
        LOG_ERROR("absence de valeur position \"%i\" dans cpu->memory_handler", *(int*)i);
        return NULL;
    }

    return val; // retourne le pointeur vers la valeur
}

void handle_MOV(CPU *cpu, void *src, void *dest){
    memcpy(dest, src, sizeof(int)); // Copie de la valeur source vers la destination
}

void *resolve_adressing(CPU *cpu,const char *operand){
    // renvoie la valeur de l'opérande selon le mode d'adressage
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

    return NULL; // Aucune correspondance trouvée
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
        // On vérifie si l'opérande 2 est vide (c'est-à-dire un label)
        if(result->code_instructions[i]->operand2[0] == '\0'){
            // On récupère le numéro de l'instruction associée au label
            void* val = hashmap_get(result->labels, result->code_instructions[i]->operand1);
            if(val==NULL){
                LOG_ERROR("Label \"%s\" non trouvé", result->code_instructions[i]->operand1);
                return 0;
            }
            // Conversion de l'adresse en chaîne de caractères
            char str[20];
            sprintf(str, "%d", *(int*)val);
            // On remplace l'opérande 1 par le numéro de l'instruction
            free(result->code_instructions[i]->operand1);
            result->code_instructions[i]->operand1 = strdup(str);
        }else if(search_and_replace(&result->code_instructions[i]->operand2, result->memory_locations) == 0){
            // Si le remplacement de l'opérande 2 associé à son addresse échoue
            LOG_ERROR("operand \"%s\" non trouvé", result->code_instructions[i]->operand2);
            return 0;
        } 
    }
    return 1;
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count){
    // Récupération du segment de données
    Segment *ds = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    // Création du segment de code (Code Segment) positionné après le segment de données
    if(create_segment(cpu->memory_handler,"CS",ds->size,code_count)==0){
        LOG_ERROR("Echec de création_segment");
        return;
    }
    // Stockage des instructions dans le segment de code
    for(int i = 0; i<code_count; i++){
        // On duplique l'instruction
        Instruction *instr = (Instruction *)malloc(sizeof(Instruction));
        instr->mnemonic = strdup(code_instructions[i]->mnemonic);
        instr->operand1 = strdup(code_instructions[i]->operand1);
        instr->operand2 = strdup(code_instructions[i]->operand2);
        // On stocke l'instruction dans le segment de code
        store(cpu->memory_handler, "CS", i, instr);
    }
    // On initialise le registre IP à 0 (début du segment de code)
    void* ip = hashmap_get(cpu->context, "IP");
    *(int*)ip = 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest){
    // Application de l'instruction selon le mnemonic (retourne 1 si executé avec success, 0 sinon)
    if(strcmp(instr->mnemonic, "MOV")==0){
        handle_MOV(cpu, src, dest);
        return 1;
    }else if(strcmp(instr->mnemonic, "ADD")==0){
        *(int*)dest += *(int*)src;  // Additionne la valeur source à la destination
        return 1;
    }else if(strcmp(instr->mnemonic, "CMP")==0){
        int cmp = *(int*)dest-*(int*)src; // Compare la valeur source à la destination
        // On met à jour les flags ZF et SF selon le résultat de la comparaison
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
        void* ip = hashmap_get(cpu->context, "IP"); // Instruction Pointer
        // On met à jour l'IP avec la valeur de l'opérande 1
        *(int*)ip = *(int*)resolve_adressing(cpu, instr->operand1);

        return 1;
    }else if(strcmp(instr->mnemonic, "JZ")==0){
        void* zf = hashmap_get(cpu->context, "ZF"); // Zero Flag
        // On vérifie si le flag ZF est égal à 1 (vérification de l'égalité)
        if(*(int*)zf == 1){
            void* ip = hashmap_get(cpu->context, "IP"); // Instruction Pointer
            *(int*)ip = *(int*)resolve_adressing(cpu, instr->operand1); // On met à jour l'IP avec la valeur de l'opérande 1
        }
        return 1;
    }else if(strcmp(instr->mnemonic, "JNZ") == 0){
        void* zf = hashmap_get(cpu->context, "ZF"); // Zero Flag
        // On vérifie si le flag ZF est égal à 0 (vérification de l'inégalité)
        if(*(int*)zf == 0){
            void* ip = hashmap_get(cpu->context, "IP"); // Instruction Pointer
            *(int*)ip = *(int*)resolve_adressing(cpu, instr->operand1); // On met à jour l'IP avec la valeur de l'opérande 1
        }
        return 1;
    }else if(strcmp(instr->mnemonic, "HALT")==0){
        void* ip = hashmap_get(cpu->context, "IP"); // Instruction Pointer
        // On récupère le segment de code (CS)
        Segment *cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
        // On met à jour l'IP avec la taille du segment de code (fin de l'exécution)
        *(int*)ip = cs->size;
        return 1; 
    }

    return 0;
}

int execute_instruction(CPU *cpu, Instruction *instr){
    void* src =  resolve_adressing(cpu, instr->operand2); // On résout l'adressage de l'opérande 2
    void* dest =  resolve_adressing(cpu, instr->operand1); // On résout l'adressage de l'opérande 1
    return handle_instruction(cpu, instr, src, dest); // On exécute l'instruction
}

Instruction *fetch_next_instruction(CPU *cpu){
    void* ip = hashmap_get(cpu->context, "IP"); // Instruction Pointer
    void* inst = load(cpu->memory_handler, "CS", *(int*)ip); // On charge l'instruction à l'adresse pointée par l'IP
    *(int*)ip +=1; // On incrémente l'IP pour pointer vers la prochaine instruction
    return inst; // On retourne l'instruction chargée
}

void print_registers(CPU *cpu){
    // On affiche les registres du CPU
    printf("Registres :\n");
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

    printf("Execution... (entrez 'q' pour quitter, entree pour continuer)\n");
    while (1) {
        Instruction *inst = fetch_next_instruction(cpu);
        if(inst==NULL){
            break;
        }
        printf("Execution de : %s %s %s\n", inst->mnemonic, inst->operand1, inst->operand2);
        int exit_code = execute_instruction(cpu, inst);
        printf("Code de sortie : %i\n", exit_code);

        printf("> ");
        char input[10];
        fgets(input, sizeof(input), stdin);
    
        if (input[0] == 'q') {
            break;
        }
    }

    printf("--Etat Final--\n");
    print_data_segment(cpu);
    print_registers(cpu);

    return 1;
}

//int push_value(CPU *cpu, int value){
//    void *sp = hashmap_get(cpu->context, "SP");
//    if(sp == NULL){
//        return -1;
//    }
//    if(*(int*)sp==128){
//        //print stack overflow
//        return -1;
//    }
//    *(int*)sp -=1;
//}
//
//int pop_value(CPU *cpu, int *dest){
//    void *sp = hashmap_get(cpu->context, "SP");
//    if(sp == NULL){
//        return -1;
//    }
//    if(*(int*)sp==0){
//        //print stack overflow
//        return -1;
//    }
//    *(int*)sp +=1;
//}