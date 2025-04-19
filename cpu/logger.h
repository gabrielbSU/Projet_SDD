#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>

// Macro variadique pour afficher un message d'erreur sur stderr avec nom du fichier et la ligne
#define LOG_ERROR(message, ...) \
    fprintf(stderr, "ERREUR : " message " [%s:%d]\n", ##__VA_ARGS__, __FILE__, __LINE__)

// Macro pour vérifier une condition, afficher une erreur et quitter le programme si elle échoue
#define LOG_ASSERT(condition, ...) \
    if (!(condition)){ \
        LOG_ERROR("(Assertion). " __VA_ARGS__); \
        exit(EXIT_FAILURE); \
    }

#endif