#include <argp.h>
#include "libgmbar.h"

#ifndef COMMON_H
#define COMMON_H

unsigned int parse_option_arg_unsigned_int(char* arg);
int parse_option_arg_string(char* arg, char** str);

/* Common argp children */
extern const struct argp_child common_argp_child[];

/* Common argp input */
typedef struct common_arguments common_arguments;
struct common_arguments {
        gmbar* bar;
        unsigned int interval;
        char* prefix;
        char* suffix;
};

void print_bar(common_arguments* args);

#endif //COMMON_H
