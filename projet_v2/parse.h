#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hachage.h"

#define TABLE_SIZE 128
#define TOMBSTONE ((void*)-1)

typedef struct {
    char *mnemonic;
    char *operand1;
    char *operand2;
} Instruction;

typedef struct {
    Instruction **date_instructions;
    int data_count;
    Instruction **cpde_instructions;
    int code_count;
    HashMap *labels;
    HashMap *memory_locations;
} ParserResult;

Instruction *parse_data_instruction(const char *line,HashMap *memory_locations);
Instruction *parse_code_instruction(const char *line,HashMap *labels,int code_count);
ParserResult *parse(const char*filename);
void free_parser_result(ParserResult *result);
#endif 