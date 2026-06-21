#ifndef DEBUG_H
#define DEBUG_H

/* "IWYU pragma: keep" disables clangd warnings */
#include <stdio.h>  // IWYU pragma: keep
#include <stdlib.h> // IWYU pragma: keep

#define MAX_MSG_SIZE 1024

#define PANIC(_code, _msg)                                                     \
  {                                                                            \
    fprintf(stderr, "[%s:%i] %s\n", __FILE__, __LINE__, _msg);                 \
    exit(_code);                                                               \
  }

#define PANICF(_code, _msg, ...)                                               \
  {                                                                            \
    char _msg_2[MAX_MSG_SIZE];                                                 \
    snprintf(_msg_2, MAX_MSG_SIZE, "[%s:%i] %s\n", __FILE__, __LINE__, _msg);  \
    fprintf(stderr, _msg_2, __VA_ARGS__);                                      \
    exit(_code);                                                               \
  }

#define INFO(_msg) fprintf(stdout, "[%s:%i] %s\n", __FILE__, __LINE__, _msg);

#define INFOF(_msg, ...)                                                       \
  {                                                                            \
    char _msg_2[MAX_MSG_SIZE];                                                 \
    snprintf(_msg_2, MAX_MSG_SIZE, "[%s:%i] %s\n", __FILE__, __LINE__, _msg);  \
    fprintf(stdout, _msg_2, __VA_ARGS__);                                      \
  }

#endif
