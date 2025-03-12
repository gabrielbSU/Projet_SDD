#include "parse.h"

int total_data_allocated = 0;
int total_code_allocated = 0;

Instruction *parse_data_instruction(const char *line,HashMap *memory_locations){
    Instruction *new_i = (Instruction *)malloc(sizeof(Instruction));
    if(new_i == NULL){
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }
    sscanf(line,"%s %s %s",new_i->mnemonic, new_i->operand1, new_i->operand2);
    hashmap_insert(memory_locations, new_i->mnemonic,total_data_allocated);

    totalAllocated+=len(new_i->operand2)/2 +1;
    
    return new_i;
}

Instruction *parse_code_instruction(const char *line,HashMap *labels,int code_count){
    Instruction *new_i = (Instruction *)malloc(sizeof(Instruction));
    if(new_i == NULL){
        printf("Erreur d'allocation mémoire\n");
        return NULL;
    }
    char *s;
    sscanf(line,"%s",s)
    if(strcmp(s,"loop:")==0 || strcmp(s,"end:")==0){
        sscanf(line,"%s %s %s %s",s ,new_i->mnemonic,new_i->operand1,new_i->operand2);
        hashmap_insert(memory_locations,s,total_code_allocated);
    }else{
        sscanf(line,"%s %s %s",new_i->mnemonic,new_i->operand1,new_i->operand2);
    }
    total_code_allocated += 1;
    
    return new_i;
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
    char buffer[64];
    int is_in_data = 0;
    int is_in_code = 0;

    fgets(buffer,sizeof(buffer),f);
    while(fgets(buffer,sizeof(buffer),f)){
        if(strcmp(buffer,".DATA") == 0){
            is_in_data = 1;
            is_in_code = 0;
        }
        if(is_in_data == 1){
            new_r->data_count += 1;
        }

        if(strcmp(buffer,".CODE") == 0){
            is_in_code = 1;
            is_in_data = 0;
        }
        if(is_in_code == 1){
            new_r->code_count += 1;
        }
    }
    new_r->data_count-= 1;
    new_r->code_count-= 1;

    //new_r->date_instructions = (Instruction **) malloc(*sizeof(Instruction*))

    return new_r;
}
