#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "libgmbar.h"
#include "common.h"
#include "log.h"
#include "readfile.h"
#include "version.h"

/* Static functions */
static error_t handle_option(int key,
                             char* arg,
                             struct argp_state *state);
static int get_meminfo(unsigned int *total,
                       unsigned int *used,
                       unsigned int *buffers,
                       unsigned int *cached);
static int parse_meminfo(const char* meminfo,
                         unsigned int *total,
                         unsigned int *used,
                         unsigned int *buffers,
                         unsigned int *cached);
static int parse_meminfo_field(const char* meminfo,
                               const char* field,
                               unsigned int *value);
static unsigned int parse_unsigned_int(const char* str);

/* Argp option keys */
enum {
        OPTION_USED_COLOR = 'a',
        OPTION_BUFFERS_COLOR = 'b',
        OPTION_CACHED_COLOR = 'c',
};


/* Argp input */
typedef struct arguments arguments;
struct arguments {
        common_arguments common_config;
};

/* Options */
static struct argp_option options[] = {
        { "used",       OPTION_USED_COLOR,         "COLOR",     0,
          "Color for the memory used portion of the bar"        },
        { "buffers",    OPTION_BUFFERS_COLOR,      "COLOR",     0,
          "Color for the buffers portion of the bar"            },
        { "cached",     OPTION_CACHED_COLOR,       "COLOR",     0,
          "Color for the disk cache portion of the bar"         },
        { 0 }
};

/* Argp parser */
static const struct argp argp = { options, handle_option, NULL,
                                  "gmmembar -- dzen2 memory bar",
                                  common_argp_child
};

int
main(int argc, char** argv)
{
        int err = 0;
        unsigned int total, used, buffers, cached;
        arguments config;
        gmbar* bar = NULL;
        char* buf = NULL;

        bar = gmbar_new_with_defaults(100, 10, "red", "#444444");
        if (!bar)
        {
                return -1;
        }

        err = gmbar_add_sections(bar, 3, "red", "orange", "yellow");
        if (err)
        {
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
                gmbar_free(bar);
                return err;
        }

        do {
                err = get_meminfo(&total, &used, &buffers, &cached);
                if (err)
                {
                        gmbar_free(bar);
                        return err;
                }

                gmbar_set_section_width(bar->sections[0], total, used);
                gmbar_set_section_width(bar->sections[1], total, buffers);
                gmbar_set_section_width(bar->sections[2], total, cached);

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

                sleep(config.common_config.interval);
        } while (config.common_config.interval);

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

        case OPTION_USED_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[0]->color);
                break;
        case OPTION_BUFFERS_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[1]->color);
                break;
        case OPTION_CACHED_COLOR:
                err = parse_option_arg_string(arg, &config->common_config.bar->sections[2]->color);
                break;

        default:
                err = ARGP_ERR_UNKNOWN;
                break;
        }

        return err;
}

static int
get_meminfo(unsigned int *total,
            unsigned int *used,
            unsigned int *buffers,
            unsigned int *cached)
{
        int err = 0;
        char* meminfo = NULL;

        *total = *used = *buffers = *cached = 0;

        meminfo = readfile("/proc/meminfo");
        if (!meminfo)
        {
                return errno;
        }

        err = parse_meminfo(meminfo, total, used, buffers, cached);
        free(meminfo);
        return err;
}


static int
parse_meminfo(const char* meminfo,
              unsigned int *total,
              unsigned int *used,
              unsigned int *buffers,
              unsigned int *cached)
{
        int err = 0;
        unsigned int free;

        err = parse_meminfo_field(meminfo, "MemTotal", total);
        if (err)
        {
                log_error("Error parsing MemTotal: %d\n", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, "MemFree", &free);
        if (err)
        {
                log_error("Error parsing MemFree: %d\n", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, "Buffers", buffers);
        if (err)
        {
                log_error("Error parsing Buffers: %d\n", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, "Cached", cached);
        if (err)
        {
                log_error("Error parsing Cached: %d\n", err);
                return err;
        }

        *used = *total - (free + *buffers + *cached);

        return err;
}

/**
 * Parse one field of meminfo.
 *
 * @param   meminfo   Contents of the /proc/meminfo file
 * @param   field     Name of the field to parse, e.g. "MemTotal", zero terminated
 * @param   value     On return, contains the parsed value or zero.
 * @return  Zero on success, -1 if the field is not found.  Note that any
 * parse errors are not detected.
 */
static int
parse_meminfo_field(const char* meminfo,
                    const char* field,
                    unsigned int *value)
{
        const char* p = meminfo;

        /* Initialize to zero */
        *value = 0;

        /* Find the field */
        do
        {
                p = strstr(p, field);
                if (!p)
                {
                        return -1;
                }
        } while (p != meminfo && p[-1] != '\n' && p++);

        /* Skip the label */
        p += strlen(field);

        /* Skip non-digits like colons and spaces */
        while (!isdigit(*p))
                p++;

        /* Parse the digits (base ten value) */
        *value = parse_unsigned_int(p);

        /* meminfo is in kilobytes */
        *value = *value * 1024;

        return 0;
}

/**
 * @param   str   String to parse
 * @return  Parsed value.
 */
static unsigned int
parse_unsigned_int(const char* str)
{
        unsigned int value = 0;
        const char *p = str;
        while(isdigit(*p))
                value = value * 10 + (*p++ - '0');
        return value;
}
