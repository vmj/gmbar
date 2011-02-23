#include <stdio.h>
#include <errno.h>

#include "log.h"


static FILE* log = NULL;

/**
 *
 */
int log_open(char* filename)
{
        int err = 0;
        if (log)
        {
                err = log_close();
        }
        if (!err)
        {
                log = fopen(filename, "a");
                if (!log)
                {
                        err = errno;
                }
        }
        return err;
}

/**
 *
 */
int log_close()
{
        int err = 0;
        if (log)
        {
                err = fclose(log);
                /* error or not, log should not be accessed */
                log = NULL;
        }
        return err == EOF ? errno : 0;
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
