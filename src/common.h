#include <argp.h>
#include "libgmbar.h"

#ifndef COMMON_H
#define COMMON_H

int parse_option_arg_unsigned_int(char* arg, unsigned int* val);
int parse_option_arg_unsigned_char(char* arg, unsigned char* val);
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

const char*   memstr   (const char* data,
                        const char* str,
                        unsigned int size);

int print_bar(common_arguments* args);

#endif //COMMON_H
