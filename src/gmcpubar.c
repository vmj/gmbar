#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "libgmbar.h"
#include "common.h"
#include "log.h"
#include "readfile.h"
#include "buffer.h"
#include "version.h"

/* Static functions */
static error_t handle_option(int key,
                             char* arg,
                             struct argp_state *state);
static int get_stat(buffer* stat,
                    unsigned int *kern,
                    unsigned int *user,
                    unsigned int *nice,
                    unsigned int *idle);
static long get_num_cpus();
static long parse_cpuinfo(const char* cpuinfo,
                          const unsigned int size);
static int parse_stat(const char* stat,
                      const unsigned int size,
                      const char* field,
                      unsigned int *kern,
                      unsigned int *user,
                      unsigned int *nice,
                      unsigned int *idle);
static unsigned int parse_unsigned_int(const char* str,
                                       const char** end);


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
        buffer* stat = NULL;
        gmbar* bar = NULL;

        stat = buffer_new();
        if (!stat)
        {
                return -1;
        }

        bar = gmbar_new_with_defaults(100, 10, "red", "none");
        if (!bar)
        {
                buffer_free(stat);
                return -1;
        }

        err = gmbar_add_sections(bar, 4, "red", "orange", "yellow", "none");
        if (err)
        {
                buffer_free(stat);
                gmbar_free(bar);
                return -1;
        }

        config.common_config.bar = bar;
        config.common_config.interval = 15;
        config.common_config.prefix = NULL;
        config.common_config.suffix = NULL;
        err = argp_parse(&argp, argc, argv, 0, NULL, &config);
        if (err)
        {
                buffer_free(stat);
                gmbar_free(bar);
                return err;
        }

        /* Clock ticks per second (per CPU) */
        total = sysconf(_SC_CLK_TCK);

        /* Number of CPUs */
        num_cpus = get_num_cpus();
        if (num_cpus < 0)
        {
                buffer_free(stat);
                gmbar_free(bar);
                return num_cpus;
        }

        /* Initialize history */
        err = get_stat(stat, &_kern, &_user, &_nice, &_idle);
        if (err)
        {
                buffer_free(stat);
                gmbar_free(bar);
                return err;
        }

        while (config.common_config.interval)
        {
                sleep(config.common_config.interval);

                err = get_stat(stat, &kern, &user, &nice, &idle);
                if (err)
                {
                        buffer_free(stat);
                        gmbar_free(bar);
                        return err;
                }

                /* total is not accurate */
                total = (kern - _kern) + (user - _user) + (nice - _nice) + (idle - _idle);

                gmbar_set_section_width(bar->sections[0], total, kern - _kern);
                gmbar_set_section_width(bar->sections[1], total, user - _user);
                gmbar_set_section_width(bar->sections[2], total, nice - _nice);
                gmbar_set_section_width(bar->sections[3], total, idle - _idle);

                err = print_bar(&config.common_config);
                if (err)
                {
                        buffer_free(stat);
                        gmbar_free(bar);
                        return err;
                }

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
get_stat(buffer* stat,
         unsigned int *kern,
         unsigned int *user,
         unsigned int *nice,
         unsigned int *idle)
{
        int err = 0;

        *kern = *user = *nice = *idle = 0;

        /* reuse all the space */
        stat->len = 0;

        err = readfile("/proc/stat", &stat->buf, &stat->len, &stat->max);
        if (err)
        {
                return err;
        }

        err = parse_stat(stat->buf, stat->len, "cpu ", kern, user, nice, idle);
        return err;
}


static long
get_num_cpus()
{
        int err = 0;
        char* cpuinfo = NULL;
        unsigned int size = 0;
        unsigned int max = 0;

        err = readfile("/proc/cpuinfo", &cpuinfo, &size, &max);
        if (err || !cpuinfo)
        {
                return err;
        }

        return parse_cpuinfo(cpuinfo, size);
}

/**
 *
 */
static long
parse_cpuinfo(const char* cpuinfo, const unsigned int size)
{
        const char* p = cpuinfo;
        unsigned int len = size;
        long cpus = 0;

        do
        {
                p = memstr(p, "processor", len);
                if (p && (p == cpuinfo || p[-1] == '\n'))
                {
                        cpus++;
                        len = size - (p - cpuinfo);
                        p++;
                }
        } while (p);

        return cpus;
}

/**
 * Parse CPU field from stat.
 *
 * @param   stat      Contents of the /proc/stat file
 * @param   size      Size of the content
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
           const unsigned int size,
           const char* field,
           unsigned int *kern,
           unsigned int *user,
           unsigned int *nice,
           unsigned int *idle)
{
        const char* p = stat;
        unsigned int len = size;

        /* Initialize to zeros */
        *kern = *user = *nice = *idle = 0;

        /* Find the field */
        do
        {
                p = memstr(p, field, len);
                if (!p)
                {
                        log_error("Field not found: %d", -1);
                        return -1;
                }
        } while (p != stat && p[-1] != '\n' && p++ && (len = size - (p - stat)));

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
parse_unsigned_int(const char* str, const char** end)
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
