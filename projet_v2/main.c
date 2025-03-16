#include "parse.h"

int main(){
    ParserResult *res = parse("code.asm");

    printf("start -> %i\n", *(int*)hashmap_get(res->labels, "start"));
    printf("loop -> %i\n\n",*(int*)hashmap_get(res->labels, "loop"));

    printf("x -> %i\n",  *(int*)hashmap_get(res->memory_locations, "x"));
    printf("arr -> %i\n",*(int*)hashmap_get(res->memory_locations, "arr"));
    printf("y -> %i\n",  *(int*)hashmap_get(res->memory_locations, "y"));

    return 0;
}