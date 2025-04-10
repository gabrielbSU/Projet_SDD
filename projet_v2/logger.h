#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define LOG_ERROR(Message, ...) \
    fprintf(stderr, "ERREUR : " Message " [%s:%d]\n", ##__VA_ARGS__, __FILE__, __LINE__)

#endif