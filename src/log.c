#include <stdio.h>
#include <errno.h>

#include "log.h"


static FILE* log = NULL;

/**
 *
 */
int log_open(char* filename)
{
        log = fopen(filename, "a");
        return errno;
}

/**
 *
 */
void log_error(char* frmt, int err)
{
        if (log)
        {
                fprintf(log, frmt, err);
                fflush(log);
        }
}
