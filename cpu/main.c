#include <stdlib.h>
#include "parser.h"
#include "cpu.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    // Verifie qu'un chemin a bien ete fourni en argument
    LOG_ASSERT(argc == 2, "veuillez fournir le chemin du fichier assembleur en argument");

    // Analyse le fichier source assembleur
    ParserResult *res = parse(argv[1]);

    // Initialise le CPU
    CPU *cpu = cpu_init(256 * 8);
    
    // Resolution des constantes
    int ret_code = resolve_constants(res);
    LOG_ASSERT(ret_code != 0, "echec de la resolution des constantes");
    
    // Alloue les donnees et le code dans la memoire du CPU
    allocate_variables(cpu, res->data_instructions, res->data_count);
    allocate_code_segment(cpu, res->code_instructions, res->code_count);

    // Libere le resultat du parseur
    free_parser_result(res);

    // Execute le programme
    run_program(cpu);

    // Libere la memoire du CPU
    cpu_destroy(cpu);

    return 0;
}
