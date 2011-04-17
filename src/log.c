#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "log.h"

extern char* program_invocation_short_name;

static FILE* log = NULL;
static char hostname[256];
static char datetime[25];

static void log_prefix();
static void log_suffix();

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
        if (!err)
        {
                err = gethostname(hostname, 255);
                if (err)
                {
                        hostname[0] = '\0';
                        log_error("Unable to get hostname: %d", errno);
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
void log_error(char* frmt, ...)
{
        va_list argv;
        if (log)
        {
                log_prefix();

                va_start(argv, frmt);
                vfprintf(log, frmt, argv);
                va_end(argv);

                log_suffix();
        }
}

/**
 * Tries to output "DATE HOSTNAME PROGRAM: " into log.
 */
void log_prefix()
{
        if (log)
        {
                const time_t _t = time(NULL);
                const struct tm* t = localtime(&_t);
                if (t)
                {
                        //Apr 16 12:15:45
                        strftime(datetime, 25, "%b %d %T", t);
                }
                else
                {
                        memcpy(datetime, "xxx xx xx:xx:xx", 25);
                }
                if (hostname[0] != '\0')
                {
                        fprintf(log, "%s %s %s: ",
                                datetime,
                                hostname,
                                program_invocation_short_name);
                }
                else
                {
                        fprintf(log, "%s %s: ",
                                datetime,
                                program_invocation_short_name);
                }
        }
}

/**
 * Outputs "\n" to the log and flushes it.
 */
void log_suffix()
{
        if (log)
        {
                fprintf(log, "\n");
                fflush(log);
        }
}
