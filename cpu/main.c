#include <stdlib.h>
#include "parser.h"
#include "cpu.h"
#include "logger.h"

int main(){
    ParserResult *res = parse("asm/code.asm");

    CPU *cpu = cpu_init(256*8);
    allocate_variables(cpu, res->data_instructions, res->data_count);
    allocate_code_segment(cpu, res->code_instructions, res->code_count);

    run_program(cpu);

    cpu_destroy(cpu);
    free_parser_result(res);

    return 0;
}