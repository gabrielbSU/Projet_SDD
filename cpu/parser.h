#ifndef PARSER_H
#define PARSER_H

#include "hachage.h"
#include "dynamique.h"

extern int total_data_allocated;

// Structure pour représenter une instruction
typedef struct {
    char *mnemonic; // Instruction mnemonic (ou nom de variable pour .DATA)
    char *operand1; // Premier operande (ou type pour .DATA)
    char *operand2; // Second operande (ou initialisation pour .DATA)
} Instruction;

// Structure pour représenter le résultat du parseur
typedef struct {
    Instruction **data_instructions; // Tableau d’instructions .DATA
    int data_count; // Nombre d’instructions .DATA
    Instruction **code_instructions; // Tableau d’instructions .CODE
    int code_count; // Nombre d’instructions .CODE
    HashMap *labels;    // labels -> indices dans code_instructions
    HashMap *memory_locations; // noms de variables -> adresse memoire
} ParserResult;

// Fonctions de parsing
Instruction *parse_data_instruction(const char *line,HashMap *memory_locations);
Instruction *parse_code_instruction(const char *line,HashMap *labels,int code_count);
ParserResult *parse(const char *filename);
void free_parser_result(ParserResult *result);

#endif 