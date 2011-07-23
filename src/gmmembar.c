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
static int get_meminfo(buffer* meminfo,
                       unsigned int *total,
                       unsigned int *used,
                       unsigned int *buffers,
                       unsigned int *cached);
static int parse_meminfo(const char* meminfo,
                         const unsigned int size,
                         unsigned int *total,
                         unsigned int *used,
                         unsigned int *buffers,
                         unsigned int *cached);
static int parse_meminfo_field(const char* meminfo,
                               const unsigned int size,
                               const char* field,
                               unsigned int *value);
static unsigned int parse_unsigned_int(const char* str);

/* Argp option keys (available: 'a'-'f') */
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
        buffer* meminfo = NULL;
        gmbar* bar = NULL;

        meminfo = buffer_new();
        if (!meminfo)
        {
                return -1;
        }

        bar = gmbar_new_with_defaults(100, 10, "red", "none");
        if (!bar)
        {
                buffer_free(meminfo);
                return -1;
        }

        err = gmbar_add_sections(bar, 3, "red", "orange", "yellow");
        if (err)
        {
                buffer_free(meminfo);
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
                buffer_free(meminfo);
                gmbar_free(bar);
                return err;
        }

        do {
                err = get_meminfo(meminfo, &total, &used, &buffers, &cached);
                if (err)
                {
                        buffer_free(meminfo);
                        gmbar_free(bar);
                        return err;
                }

                gmbar_set_section_width(bar->sections[0], total, used);
                gmbar_set_section_width(bar->sections[1], total, buffers);
                gmbar_set_section_width(bar->sections[2], total, cached);

                err = print_bar(&config.common_config);
                if (err)
                {
                        buffer_free(meminfo);
                        gmbar_free(bar);
                        return err;
                }

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
get_meminfo(buffer* meminfo,
            unsigned int *total,
            unsigned int *used,
            unsigned int *buffers,
            unsigned int *cached)
{
        int err = 0;

        *total = *used = *buffers = *cached = 0;

        /* reuse all the space */
        meminfo->len = 0;

        err = readfile("/proc/meminfo", &meminfo->buf, &meminfo->len, &meminfo->max);
        if (err)
        {
                return err;
        }

        err = parse_meminfo(meminfo->buf, meminfo->len, total, used, buffers, cached);
        return err;
}


static int
parse_meminfo(const char* meminfo,
              const unsigned int size,
              unsigned int *total,
              unsigned int *used,
              unsigned int *buffers,
              unsigned int *cached)
{
        int err = 0;
        unsigned int free;

        err = parse_meminfo_field(meminfo, size, "MemTotal", total);
        if (err)
        {
                log_error("Error parsing MemTotal: %d", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, size, "MemFree", &free);
        if (err)
        {
                log_error("Error parsing MemFree: %d", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, size, "Buffers", buffers);
        if (err)
        {
                log_error("Error parsing Buffers: %d", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, size, "Cached", cached);
        if (err)
        {
                log_error("Error parsing Cached: %d", err);
                return err;
        }

        *used = *total - (free + *buffers + *cached);

        return err;
}

/**
 * Parse one field of meminfo.
 *
 * @param   meminfo   Contents of the /proc/meminfo file
 * @param   size      Content length in bytes
 * @param   field     Name of the field to parse, e.g. "MemTotal", zero terminated
 * @param   value     On return, contains the parsed value or zero.
 * @return  Zero on success, -1 if the field is not found.  Note that any
 * parse errors are not detected.
 */
static int
parse_meminfo_field(const char* meminfo,
                    const unsigned int size,
                    const char* field,
                    unsigned int *value)
{
        const char* p = meminfo;
        unsigned int len = size;

        /* Initialize to zero */
        *value = 0;

        /* Find the field */
        do
        {
                p = memstr(p, field, len);
                if (!p)
                {
                        return -1;
                }
        } while (p != meminfo && p[-1] != '\n' && p++ && len--);

        /* Skip the label */
        p += strlen(field);

        /* Skip non-digits like colons and spaces */
        while (!isdigit(*p))
                p++;

        /* Parse the digits (base ten value) */
        *value = parse_unsigned_int(p);

        /* meminfo is in kilobytes (KiB) */
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
