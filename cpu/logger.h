#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>

#define LOG_ERROR(message, ...) \
    fprintf(stderr, "ERREUR : " message " [%s:%d]\n", ##__VA_ARGS__, __FILE__, __LINE__)

#define LOG_ASSERT(condition, ...) \
    if (!(condition)){ \
        LOG_ERROR("(Assertion). " __VA_ARGS__); \
        exit(EXIT_FAILURE); \
    }

#endif