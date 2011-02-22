#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <argp.h>

#include "libgmbar.h"
#include "common.h"
#include "version.h"

/* Static functions */
static error_t handle_option(int key,
                             char* arg,
                             struct argp_state *state);
static int get_stat(unsigned int *kern,
                    unsigned int *user,
                    unsigned int *nice,
                    unsigned int *idle);
static long get_num_cpus();
static char* readfile(const char* filepath);
static long parse_cpuinfo(const char* cpuinfo);
static int parse_stat(const char* stat,
                      const char* field,
                      unsigned int *kern,
                      unsigned int *user,
                      unsigned int *nice,
                      unsigned int *idle);
static unsigned int parse_unsigned_int(const char* str,
                                       char** end);

static FILE* log = NULL;
#define LOG(f, i) if (log) fprintf(log, f, i); fflush(log);

/* Argp option keys */
enum {
        OPTION_KERN_COLOR = 'a',
        OPTION_USER_COLOR = 'b',
        OPTION_NICE_COLOR = 'c',
        OPTION_IDLE_COLOR = 'd',
};


/* Argp input */
typedef struct arguments arguments;
struct arguments {
        common_arguments common_config;
};

/* Options */
static struct argp_option options[] = {
        { "kern",       OPTION_KERN_COLOR,         "COLOR",     0,
          "Color for the kernel portion of the bar"             },
        { "user",       OPTION_USER_COLOR,         "COLOR",     0,
          "Color for the user portion of the bar"               },
        { "nice",       OPTION_NICE_COLOR,         "COLOR",     0,
          "Color for the nice portion of the bar"               },
        { "idle",       OPTION_IDLE_COLOR,         "COLOR",     0,
          "Color for the idle portion of the bar"               },
        { 0 }
};

/* Argp parser */
static const struct argp argp = { options, handle_option, NULL,
                                  "gmcpubar -- dzen2 cpu bar",
                                  common_argp_child
};

int
main(int argc, char** argv)
{
        int err = 0;
        long total; // Number of clock ticks per second
        long num_cpus;
        unsigned int _kern, _user, _nice, _idle;
        unsigned int kern, user, nice, idle;
        arguments config;
        gmbar* bar = NULL;
        char* buf = NULL;

        bar = gmbar_new_with_defaults(100, 10, "red", "#444444");
        if (!bar)
        {
                return -1;
        }

        err = gmbar_add_sections(bar, 4, "red", "orange", "yellow", "green");
        if (err)
        {
                gmbar_free(bar);
                return -1;
        }

        config.common_config.bar = bar;
        config.common_config.interval = 15;
        config.common_config.log_file = NULL;
        config.common_config.prefix = NULL;
        config.common_config.suffix = NULL;
        err = argp_parse(&argp, argc, argv, 0, NULL, &config);
        if (err)
        {
                gmbar_free(bar);
                return err;
        }

        if (config.common_config.log_file)
        {
                log = fopen(config.common_config.log_file, "a");
                if (!log)
                {
                        err = errno;
                        gmbar_free(bar);
                        free(config.common_config.log_file);
                        return err;
                }
                free(config.common_config.log_file);
                config.common_config.log_file = NULL;
        }

        /* Clock ticks per second (per CPU) */
        total = sysconf(_SC_CLK_TCK);

        /* Number of CPUs */
        num_cpus = get_num_cpus();
        if (num_cpus < 0)
        {
                gmbar_free(bar);
                return num_cpus;
        }

        /* Initialize history */
        err = get_stat(&_kern, &_user, &_nice, &_idle);
        if (err)
        {
                gmbar_free(bar);
                return err;
        }

        while (config.common_config.interval)
        {
                sleep(config.common_config.interval);

                err = get_stat(&kern, &user, &nice, &idle);
                if (err)
                {
                        gmbar_free(bar);
                        return err;
                }

                /* total is not accurate */
                total = (kern - _kern) + (user - _user) + (nice - _nice) + (idle - _idle);

                gmbar_set_section_width(bar->sections[0], total, kern - _kern);
                gmbar_set_section_width(bar->sections[1], total, user - _user);
                gmbar_set_section_width(bar->sections[2], total, nice - _nice);
                gmbar_set_section_width(bar->sections[3], total, idle - _idle);

                buf = gmbar_format(bar, 0);
                if (buf)
                {
                        if (config.common_config.prefix && config.common_config.suffix)
                        {
                                printf("%s%s%s\n", config.common_config.prefix, buf, config.common_config.suffix);
                        }
                        else if (config.common_config.prefix)
                        {
                                printf("%s%s\n", config.common_config.prefix, buf);
                        }
                        else if (config.common_config.suffix)
                        {
                                printf("%s%s\n", buf, config.common_config.suffix);
                        }
                        else
                        {
                                printf("%s\n", buf);
                        }
                        free(buf);
                        buf = NULL;
                }
                else
                {
                        printf("^fg(red)^bg(black)OOF^bg()^fg()\n");
                }
                fflush(stdout);

                _kern = kern;
                _user = user;
                _nice = nice;
                _idle = idle;
        }

        return 0;
}

static error_t
handle_option(int key, char* arg, struct argp_state *state)
{
        error_t err = 0;
        arguments* config = (arguments*) state->input;

        switch (key)
        {
        case ARGP_KEY_INIT:
                state->child_inputs[0] = config;
                break;

        case OPTION_KERN_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[0]->color);
                break;
        case OPTION_USER_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[1]->color);
                break;
        case OPTION_NICE_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[2]->color);
                break;
        case OPTION_IDLE_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[3]->color);
                break;

        default:
                err = ARGP_ERR_UNKNOWN;
                break;
        }

        return err;
}

static int
get_stat(unsigned int *kern,
         unsigned int *user,
         unsigned int *nice,
         unsigned int *idle)
{
        int err = 0;
        char* stat = NULL;

        *kern = *user = *nice = *idle = 0;

        stat = readfile("/proc/stat");
        if (!stat)
        {
                return errno;
        }

        err = parse_stat(stat, "cpu ", kern, user, nice, idle);
        free(stat);
        return err;
}


static long
get_num_cpus()
{
        char* cpuinfo = NULL;

        cpuinfo = readfile("/proc/cpuinfo");
        if (!cpuinfo)
        {
                return errno;
        }

        return parse_cpuinfo(cpuinfo);
}

static char*
readfile(const char* filepath)
{
        int err = 0;
        int fd = 0;
        ssize_t bytes = -1;
        char* buf = NULL;
        char* tmp = NULL;
        unsigned int bufsize = 0;
        unsigned int bufmax = 0;

        fd = open(filepath, O_RDONLY);
        if (fd == -1)
        {
                err = errno;
                LOG("Error opening file: %d\n", err);
        }

        while (!err && bytes != 0)
        {

                if (bufsize == bufmax)
                {
                        bufmax += 1024;
                        tmp = realloc(buf, bufmax + 1);
                        if (!tmp)
                        {
                                err = errno;
                                LOG("Error allocating space for file contents: %d", err);
                                break;
                        }
                        buf = tmp;
                }
                bytes = read(fd, buf + bufsize, bufmax - bufsize);
                switch (bytes)
                {
                case -1:
                        err = errno;
                        LOG("Error reading file: %d\n", err);
                        break;
                case 0:
                        buf[bufsize + 1] = '\0';
                        break;
                default:
                        bufsize += bytes;
                        break;
                }
        }

        if (fd != -1)
        {
                fd = close(fd);
                if (fd == -1)
                {
                        err = errno;
                        LOG("Error closing file: %d\n", err);
                }
        }

        if (err)
        {
                if (buf)
                {
                        free(buf);
                        buf = NULL;
                }
                errno = err;
        }

        return buf;
}


/**
 *
 */
static long
parse_cpuinfo(const char* cpuinfo)
{
        char* p = (char*)cpuinfo;
        long cpus = 0;

        do
        {
                p = strstr(p, "processor");
                if (p && (p == cpuinfo || p[-1] == '\n'))
                {
                        cpus++;
                        p++;
                }
        } while (p);

        return cpus;
}

/**
 * Parse CPU field from stat.
 *
 * @param   stat      Contents of the /proc/stat file
 * @param   field     Name of the field to parse, e.g. "cpu" or "cpu1", zero terminated
 * @param   kern      On return, contains the parsed value or zero.
 * @param   user      On return, contains the parsed value or zero.
 * @param   nice      On return, contains the parsed value or zero.
 * @param   idle      On return, contains the parsed value or zero.
 * @return  Zero on success, -1 if the field is not found.  Note that any
 * parse errors are not detected.
 */
static int
parse_stat(const char* stat,
           const char* field,
           unsigned int *kern,
           unsigned int *user,
           unsigned int *nice,
           unsigned int *idle)
{
        char* p = (char*)stat;

        /* Initialize to zeros */
        *kern = *user = *nice = *idle = 0;

        /* Find the field */
        do
        {
                p = strstr(p, field);
                if (!p)
                {
                        LOG("Field not found: %d\n", -1);
                        return -1;
                }
        } while (p != stat && p[-1] != '\n' && p++);

        /* Skip the label */
        p += strlen(field);

        *user = parse_unsigned_int(p, &p);
        *nice = parse_unsigned_int(p, &p);
        *kern = parse_unsigned_int(p, &p);
        *idle = parse_unsigned_int(p, NULL);

        return 0;
}

/**
 * @param   str   String to parse
 * @return  Parsed value.
 */
static unsigned int
parse_unsigned_int(const char* str, char** end)
{
        unsigned int value = 0;
        while (!isdigit(*str))
                str++;
        while(isdigit(*str))
                value = value * 10 + (*str++ - '0');
        if (end)
                *end = (char*)str;
        return value;
}
