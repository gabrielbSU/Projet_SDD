#include <string.h>
#include <ctype.h>
#include "parser.h"

int total_data_allocated = 0;

Instruction *parse_data_instruction(const char *line,HashMap *memory_locations){
    Instruction *new_i = (Instruction *)malloc(sizeof(Instruction));
    int *allocated_ptr = (int *)malloc(sizeof(int));

    if(new_i == NULL || allocated_ptr == NULL){
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }

    char mnemonic[32];
    char operand1[32];
    char operand2[32];
    sscanf(line,"%s %s %s",mnemonic, operand1, operand2);
    new_i->mnemonic = strdup(mnemonic);
    new_i->operand1 = strdup(operand1);
    new_i->operand2 = strdup(operand2);

    *allocated_ptr = total_data_allocated;
    hashmap_insert(memory_locations, new_i->mnemonic,allocated_ptr);

    int nb_comma = 0;
    for(int i = 0; i<strlen(operand2); i++){
        if(operand2[i] == ','){
            nb_comma++;
        }
    }

    total_data_allocated+=nb_comma+1;
    return new_i;
}

Instruction *parse_code_instruction(const char *line,HashMap *labels,int code_count){
    Instruction *new_i = (Instruction *)malloc(sizeof(Instruction));
    if(new_i == NULL){
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }
    char token1[32];
    char token2[32];
    char token3[32];

    sscanf(line,"%s %s %s",token1, token2, token3);
    
    if(token1[strlen(token1)-1] == ':'){
        token1[strlen(token1)-1] = '\0';

        int *allocated_ptr = (int *)malloc(sizeof(int));
        if(allocated_ptr == NULL){
            printf("Erreur d'allocation mémoire\n");
            return NULL;
        }
        *allocated_ptr = code_count;

        hashmap_insert(labels,token1,allocated_ptr);

        new_i->mnemonic = strdup(token2);

        char *operand1;
        char *operand2;
        operand1 = strtok(token3, ", ");
        operand2 = strtok(NULL, ", "); 

        new_i->operand1 = strdup(operand1);
        new_i->operand2 = strdup(operand2);
    }else{
        new_i->mnemonic = strdup(token1);

        char *operand1;
        char *operand2;
        operand1 = strtok(token2, ", ");
        operand2 = strtok(NULL, ", "); 
        new_i->operand1 = strdup(operand1);
        new_i->operand2 = NULL;
        if(operand2!=NULL){
            new_i->operand2 = strdup(operand2);
        }
        
    }

    return new_i;
}

int isBlank(char *line) {
    char *ch;

    for(ch = line; *ch != '\0'; ++ch) {
        if(!isspace(*ch)) {
            return 0;
        }
    }

    return 1;
}

ParserResult *parse(const char*filename){
    ParserResult *new_r= (ParserResult *)malloc(sizeof(ParserResult));
    new_r->data_count = 0;
    new_r->code_count = 0;
    if(new_r == NULL){
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }
    FILE* f = fopen(filename, "r");
    if(f==NULL){
        printf("erreur à l'ouverture du fichier %s\n", filename);
        return NULL;
    }
    char buffer[64];
    int is_in_data = 0;
    int is_in_code = 0;

    while(fgets(buffer,sizeof(buffer),f)){
        if(isBlank(buffer)==1){
            continue;
        }

        if(strncmp(buffer,".DATA", 5) == 0){
            is_in_data = 1;
            is_in_code = 0;
            continue;
        }
        if(strncmp(buffer,".CODE", 5) == 0){
            is_in_code = 1;
            is_in_data = 0;
            continue;
        }

        if(is_in_data == 1){
            new_r->data_count += 1;
        }
        
        if(is_in_code == 1){
            new_r->code_count += 1;
        }
    }
    is_in_data = 0;
    is_in_code = 0;

    new_r->data_instructions = (Instruction **) malloc(new_r->data_count*sizeof(Instruction*));
    new_r->code_instructions = (Instruction **) malloc(new_r->code_count*sizeof(Instruction*));
    new_r->memory_locations = hashmap_create();
    new_r->labels = hashmap_create();
    Instruction** curr_code =  new_r->code_instructions;
    Instruction** curr_data =  new_r->data_instructions;
    
    fseek(f, 0, SEEK_SET);

    int code_line = 0;

    while(fgets(buffer,sizeof(buffer),f)){
        if(isBlank(buffer)==1){
            continue;
        }

        if(strncmp(buffer,".DATA", 5) == 0){
            is_in_data = 1;
            is_in_code = 0;
            continue;
        }
        if(strncmp(buffer,".CODE", 5) == 0){
            is_in_code = 1;
            is_in_data = 0;
            continue;
        }
        
        if(is_in_data == 1){
            *curr_data=parse_data_instruction(buffer, new_r->memory_locations);
            //printf("parsed data - %s %s %s\n",
            //    (*curr_data)->mnemonic,
            //    (*curr_data)->operand1,
            //    (*curr_data)->operand2);
            curr_data++;
        }
        
        if(is_in_code == 1){
            *curr_code=parse_code_instruction(buffer, new_r->labels,code_line);
            //printf("parsed code - %s %s %s\n",
            //    (*curr_code)->mnemonic,
            //    (*curr_code)->operand1,
            //    (*curr_code)->operand2);
            curr_code++;
            code_line++;
            
        }
    }
    fclose(f);
    return new_r;
}

void free_instruction(Instruction* inst){
    free(inst->mnemonic);
    free(inst->operand1);
    free(inst->operand2);
    free(inst);
}

void free_parser_result(ParserResult *result){
    for(int i = 0; i<result->data_count;  i++){
        free_instruction(result->data_instructions[i]);
    }
    free(result->data_instructions);
    for(int i = 0; i<result->code_count;  i++){
        free_instruction(result->code_instructions[i]);
    }
    free(result->code_instructions);

    hashmap_destroy(result->memory_locations);
    hashmap_destroy(result->labels);

    free(result);
}