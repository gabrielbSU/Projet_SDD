#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "logger.h"

int total_data_allocated = 0; // compteur de la taille totale des donnees

Instruction *parse_data_instruction(const char *line,HashMap *memory_locations){
    // Allocation de la structure Instruction
    Instruction *new_i = (Instruction *)malloc(sizeof(Instruction));
    int *allocated_ptr = (int *)malloc(sizeof(int));

    LOG_ASSERT(!(new_i == NULL || allocated_ptr == NULL), "echec d'allocation memoire");

    char mnemonic[32];
    char operand1[32];
    char operand2[32];
    // Extraction des champs de l'instruction
    // On suppose que l'instruction est de la forme "mnemonic operand1 operand2"
    sscanf(line,"%s %s %s",mnemonic, operand1, operand2);
    new_i->mnemonic = strdup(mnemonic);
    new_i->operand1 = strdup(operand1);
    new_i->operand2 = strdup(operand2);

    
    *allocated_ptr = total_data_allocated; // On stocke l'adresse de la donnee
    hashmap_insert(memory_locations, new_i->mnemonic,allocated_ptr); // On insere dans le hashmap

    // On compte le nombre de virgules dans l'operande 2 (c'est-à-dire le nombre de donnees)
    // On suppose que l'operande 2 est de la forme "data1,data2,data3..."
    int nb_comma = 0;
    for(int i = 0; i<strlen(operand2); i++){
        if(operand2[i] == ','){
            nb_comma++;
        }
    }

    total_data_allocated+=nb_comma+1; // On met à jour le compteur de la taille totale des donnees
    return new_i; // On retourne l'instruction
}

Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count) {
    char l[64];
    strncpy(l, line, sizeof(l));
    l[sizeof(l) - 1] = '\0'; // Securite buffer overflow

    // Nettoyage : on enleve les \r et \n en fin de ligne
    size_t len = strlen(l);
    while (len > 0 && (l[len - 1] == '\n' || l[len - 1] == '\r')) {
        l[--len] = '\0';
    }

    // Extraction des champs de l'instruction
    char *token = strtok(l, " ");

    if (token == NULL) return NULL;

    // Gestion du label
    if (token[strlen(token) - 1] == ':') {
        token[strlen(token) - 1] = '\0'; // Supprime le ':'

        int *allocated_ptr = malloc(sizeof(int));
        LOG_ASSERT(allocated_ptr != NULL, "echec d'allocation memoire");
        *allocated_ptr = code_count;
        hashmap_insert(labels, token, allocated_ptr); // On insere le code_count dans les labels

        token = strtok(NULL, " "); // mnemonic apres le label
    }

    if (token == NULL) {
        return NULL;
    }

    // On alloue la structure Instruction
    Instruction *new_i = malloc(sizeof(Instruction));
    LOG_ASSERT(new_i != NULL, "echec d'allocation memoire");

    new_i->mnemonic = strdup(token); // Copie le mnemonic

    // Operande 1
    token = strtok(NULL, " ,");
    new_i->operand1 = token!=NULL ? strdup(token) : strdup(""); // Copie l'operande 1

    // Operande 2
    token = strtok(NULL, " ,");
    new_i->operand2 = token!=NULL ? strdup(token) : strdup(""); // Copie l'operande 2

    return new_i;
}

int is_blank(char *line) {
    // Renvoie 1 si la ligne est vide, 0 sinon
    char *ch;

    for(ch = line; *ch != '\0'; ++ch) {
        if(!isspace(*ch)) {
            return 0;
        }
    }

    return 1;
}

ParserResult *parse(const char *filename){
    // On alloue la structure ParserResult
    ParserResult *new_r= (ParserResult *)malloc(sizeof(ParserResult));
    new_r->data_count = 0;
    new_r->code_count = 0;
    LOG_ASSERT(new_r != NULL, "echec d'allocation memoire");

    // On ouvre le fichier
    FILE* f = fopen(filename, "r");
    LOG_ASSERT(f != NULL, "echec à l'ouverture du fichier \"%s\"", filename);

    char buffer[64];
    int is_in_data = 0; // Flag pour savoir si on est dans la section .DATA
    int is_in_code = 0; // Flag pour savoir si on est dans la section .CODE

    // On compte le nombre d'instructions dans le fichier
    while(fgets(buffer,sizeof(buffer),f)){
        if(is_blank(buffer)==1){ // Si la ligne est vide, on l'ignore
            continue;
        }

        if(strncmp(buffer,".DATA", 5) == 0){ // Si on trouve la section .DATA
            is_in_data = 1;
            is_in_code = 0;
            continue;
        }
        if(strncmp(buffer,".CODE", 5) == 0){ // Si on trouve la section .CODE
            is_in_code = 1;
            is_in_data = 0;
            continue;
        }

        if(is_in_data == 1){
            new_r->data_count += 1; // On compte le nombre d'instructions dans la section .DATA
        }
        
        if(is_in_code == 1){
            new_r->code_count += 1; // On compte le nombre d'instructions dans la section .CODE
        }
    }
    is_in_data = 0;
    is_in_code = 0;

    // On alloue les tableaux d'instructions
    new_r->data_instructions = (Instruction **) malloc(new_r->data_count*sizeof(Instruction*));
    new_r->code_instructions = (Instruction **) malloc(new_r->code_count*sizeof(Instruction*));
    
    LOG_ASSERT(!(new_r->data_instructions==NULL || new_r->code_instructions == NULL),
        "echec d'allocation memoire"
    );

    // On initialise les tables de hachage pour les labels et les emplacements de memoire
    new_r->memory_locations = hashmap_create();
    new_r->labels = hashmap_create();
    
    // On initialise les instructions
    Instruction** curr_code =  new_r->code_instructions;
    Instruction** curr_data =  new_r->data_instructions;
    
    fseek(f, 0, SEEK_SET); // On remet le curseur au debut du fichier

    int code_line = 0; // Compteur de ligne pour les instructions de code

    // On parse le fichier
    while(fgets(buffer,sizeof(buffer),f)){
        if(is_blank(buffer)==1){ // Si la ligne est vide, on l'ignore
            continue;
        }

        if(strncmp(buffer,".DATA", 5) == 0){ // Si on trouve la section .DATA
            is_in_data = 1;
            is_in_code = 0;
            continue;
        }
        if(strncmp(buffer,".CODE", 5) == 0){ // Si on trouve la section .CODE
            is_in_code = 1;
            is_in_data = 0;
            continue;
        }
        
        if(is_in_data == 1){
            // On parse l'instruction de donnees
            *curr_data=parse_data_instruction(buffer, new_r->memory_locations);
            //printf("parsed data - %s %s %s\n",
            //    (*curr_data)->mnemonic,
            //    (*curr_data)->operand1,
            //    (*curr_data)->operand2);
            curr_data++;
        }
        
        if(is_in_code == 1){
            // On parse l'instruction de code
            *curr_code=parse_code_instruction(buffer, new_r->labels,code_line);
            //printf("parsed code - %s %s %s\n",
            //    (*curr_code)->mnemonic,
            //    (*curr_code)->operand1,
            //    (*curr_code)->operand2);
            curr_code++;
            code_line++;
            
        }
    }
    fclose(f); // On ferme le fichier
    return new_r;
}

void free_instruction(Instruction* inst){
    // Libération de la mémoire allouée pour l'instruction
    free(inst->mnemonic);
    free(inst->operand1);
    free(inst->operand2);
    free(inst);
}

void free_parser_result(ParserResult *result){
    // Libération de la mémoire allouée pour le résultat du parseur
    for(int i = 0; i<result->data_count;  i++){
        // Libération de chaque instruction de données
        free_instruction(result->data_instructions[i]);
    }
    free(result->data_instructions);
    for(int i = 0; i<result->code_count;  i++){
        // Libération de chaque instruction de code
        free_instruction(result->code_instructions[i]);
    }
    free(result->code_instructions);

    // Libération de la mémoire allouée pour les tables de hachage
    hashmap_destroy(result->memory_locations);
    hashmap_destroy(result->labels);

    free(result);
}