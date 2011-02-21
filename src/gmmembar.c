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


/* See info libc -> Argp Global Variables */
const char * argp_program_version = "0.1";
const char * argp_program_bug_address = "vmj@linuxbox.fi";

/* Static functions */
static error_t handle_option(int key,
                             char* arg,
                             struct argp_state *state);
static int get_meminfo(unsigned int *total,
                       unsigned int *used,
                       unsigned int *buffers,
                       unsigned int *cached);
static char* readfile(const char* filepath);
static int parse_meminfo(const char* meminfo,
                         unsigned int *total,
                         unsigned int *used,
                         unsigned int *buffers,
                         unsigned int *cached);
static int parse_meminfo_field(const char* meminfo,
                               const char* field,
                               unsigned int *value);
static unsigned int parse_unsigned_int(const char* str);

static FILE* log = NULL;
#define LOG(f, i) if (log) fprintf(log, f, i); fflush(log);

/* Argp option keys */
enum {
        /* */
        OPTION_MARGIN_TOP = 0,
        OPTION_MARGIN_RIGHT = 1,
        OPTION_MARGIN_BOTTOM = 2,
        OPTION_MARGIN_LEFT = 3,
        OPTION_PADDING_TOP = 4,
        OPTION_PADDING_RIGHT = 5,
        OPTION_PADDING_BOTTOM = 6,
        OPTION_PADDING_LEFT = 7,

        /* */
        OPTION_WIDTH = 'w',
        OPTION_HEIGHT = 'h',
        OPTION_FOREGROUND_COLOR = 'F',
        OPTION_BACKGROUND_COLOR = 'B',
        OPTION_MARGIN = 'm',
        OPTION_PADDING = 'p',
        OPTION_USED_COLOR = 'a',
        OPTION_BUFFERS_COLOR = 'b',
        OPTION_CACHED_COLOR = 'c',
        OPTION_UPDATE_INTERVAL = 'i',
        OPTION_LOG_FILE = 'L',
        OPTION_PREFIX = 'P',
        OPTION_SUFFIX = 'S',
};


/* Argp input */
typedef struct arguments arguments;
struct arguments {
        gmbar* bar;
        unsigned int interval;
        char* log_file;
        char* prefix;
        char* suffix;
};

/* Options */
static struct argp_option options[] = {
        { "width",      OPTION_WIDTH,              "WIDTH",     0,
          "Width of the bar in pixels, including margins"       },
        { "height",     OPTION_HEIGHT,             "HEIGHT",    0,
          "Height of the bar in pixels, including margins"      },
        { "fg",         OPTION_FOREGROUND_COLOR,   "COLOR",     0,
          "Foreground color of the bar (the outline color)"     },
        { "bg",         OPTION_BACKGROUND_COLOR,   "COLOR",     0,
          "Background color of the bar (not used at the moment)"},
        { "margin",     OPTION_MARGIN,             "MARGIN",    0,
          "Margin size in pixels, all four sides"               },
        { "padding",    OPTION_PADDING,            "PADDING",   0,
          "Padding size in pizels, all four sides"              },
        { "used",       OPTION_USED_COLOR,         "COLOR",     0,
          "Color for the memory used portion of the bar"        },
        { "buffers",    OPTION_BUFFERS_COLOR,      "COLOR",     0,
          "Color for the buffers portion of the bar"            },
        { "cached",     OPTION_CACHED_COLOR,       "COLOR",     0,
          "Color for the disk cache portion of the bar"         },
        { "interval",   OPTION_UPDATE_INTERVAL,    "SECONDS",   0,
          "Polling intetrval in seconds (zero disables polling)"},
        { "logfile",    OPTION_LOG_FILE,           "LOGFILE",   0,
          "Log debug messages to file"                          },
        { "prefix",     OPTION_PREFIX,             "PREFIX",    0,
          "Prefix to print before the bar"                      },
        { "suffix",     OPTION_SUFFIX,             "SUFFIX",    0,
          "Suffix to print after the bar"                       },
        { 0 }
};

/* Argp parser */
static const struct argp argp = { options, handle_option, NULL,
        "gmmembar -- dzen2 memory bar"
};

int
main(int argc, char** argv)
{
        int err = 0;
        unsigned int total, used, buffers, cached;
        arguments config;
        gmbar* bar = NULL;
        char* buf = NULL;

        bar = gmbar_new();
        if (!bar)
        {
                return -1;
        }

        bar->size.width = 100;
        bar->size.height = 10;

        bar->color.fg = strdup("red");
        if (!bar->color.fg)
        {
                gmbar_free(bar);
                return -1;
        }

        bar->color.bg = strdup("#444444");
        if (!bar->color.bg)
        {
                gmbar_free(bar);
                return -1;
        }

        err = gmbar_add_section(bar, strdup("red"));
        if (err)
        {
                gmbar_free(bar);
                return -1;
        }
        err = gmbar_add_section(bar, strdup("orange"));
        if (err)
        {
                gmbar_free(bar);
                return -1;
        }
        err = gmbar_add_section(bar, strdup("yellow"));
        if (err)
        {
                gmbar_free(bar);
                return -1;
        }

        config.bar = bar;
        config.interval = 15;
        config.log_file = NULL;
        config.prefix = NULL;
        config.suffix = NULL;
        err = argp_parse(&argp, argc, argv, 0, NULL, &config);
        if (err)
        {
                gmbar_free(bar);
                return err;
        }

        if (config.log_file)
        {
                log = fopen(config.log_file, "a");
                if (!log)
                {
                        err = errno;
                        gmbar_free(bar);
                        free(config.log_file);
                        return err;
                }
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
                        if (config.prefix && config.suffix)
                        {
                                printf("%s%s%s\n", config.prefix, buf, config.suffix);
                        }
                        else if (config.prefix)
                        {
                                printf("%s%s\n", config.prefix, buf);
                        }
                        else if (config.suffix)
                        {
                                printf("%s%s\n", buf, config.suffix);
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

                sleep(config.interval);
        } while (config.interval);

        return 0;
}

static error_t
handle_option(int key, char* arg, struct argp_state *state)
{
        error_t err = 0;
        unsigned int int_value = 0;
        arguments* config = (arguments*) state->input;

        switch (key)
        {
        case OPTION_MARGIN_TOP:
                int_value = parse_unsigned_int(arg);
                config->bar->margin.top = int_value;
                break;
        case OPTION_MARGIN_RIGHT:
                int_value = parse_unsigned_int(arg);
                config->bar->margin.right = int_value;
                break;
        case OPTION_MARGIN_BOTTOM:
                int_value = parse_unsigned_int(arg);
                config->bar->margin.bottom = int_value;
                break;
        case OPTION_MARGIN_LEFT:
                int_value = parse_unsigned_int(arg);
                config->bar->margin.left = int_value;
                break;
        case OPTION_PADDING_TOP:
                int_value = parse_unsigned_int(arg);
                config->bar->padding.top = int_value;
                break;
        case OPTION_PADDING_RIGHT:
                int_value = parse_unsigned_int(arg);
                config->bar->padding.right = int_value;
                break;
        case OPTION_PADDING_BOTTOM:
                int_value = parse_unsigned_int(arg);
                config->bar->padding.bottom = int_value;
                break;
        case OPTION_PADDING_LEFT:
                int_value = parse_unsigned_int(arg);
                config->bar->padding.left = int_value;
                break;
        case OPTION_WIDTH:
                int_value = parse_unsigned_int(arg);
                config->bar->size.width = int_value;
                break;
        case OPTION_HEIGHT:
                int_value = parse_unsigned_int(arg);
                config->bar->size.height = int_value;
                break;
        case OPTION_FOREGROUND_COLOR:
                if (config->bar->color.fg)
                {
                        free(config->bar->color.fg);
                        config->bar->color.fg = NULL;
                }
                config->bar->color.fg = strdup(arg);
                if (!config->bar->color.fg)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_BACKGROUND_COLOR:
                if (config->bar->color.bg)
                {
                        free(config->bar->color.bg);
                        config->bar->color.bg = NULL;
                }
                config->bar->color.bg = strdup(arg);
                if (!config->bar->color.bg)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_MARGIN:
                int_value = parse_unsigned_int(arg);
                config->bar->margin.top = int_value;
                config->bar->margin.right = int_value;
                config->bar->margin.bottom = int_value;
                config->bar->margin.left = int_value;
                break;
        case OPTION_PADDING:
                int_value = parse_unsigned_int(arg);
                config->bar->padding.top = int_value;
                config->bar->padding.right = int_value;
                config->bar->padding.bottom = int_value;
                config->bar->padding.left = int_value;
                break;
        case OPTION_USED_COLOR:
                if (config->bar->sections[0]->color)
                {
                        free(config->bar->sections[0]->color);
                        config->bar->sections[0]->color = NULL;
                }
                config->bar->sections[0]->color = strdup(arg);
                if (!config->bar->sections[0]->color)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_BUFFERS_COLOR:
                if (config->bar->sections[1]->color)
                {
                        free(config->bar->sections[1]->color);
                        config->bar->sections[1]->color = NULL;
                }
                config->bar->sections[1]->color = strdup(arg);
                if (!config->bar->sections[1]->color)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_CACHED_COLOR:
                if (config->bar->sections[2]->color)
                {
                        free(config->bar->sections[2]->color);
                        config->bar->sections[2]->color = NULL;
                }
                config->bar->sections[2]->color = strdup(arg);
                if (!config->bar->sections[2]->color)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_UPDATE_INTERVAL:
                int_value = parse_unsigned_int(arg);
                config->interval = int_value;
                break;
        case OPTION_LOG_FILE:
                if (config->log_file)
                {
                        free(config->log_file);
                        config->log_file = NULL;
                }
                config->log_file = strdup(arg);
                if (!config->log_file)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_PREFIX:
                if (config->prefix)
                {
                        free(config->prefix);
                        config->prefix = NULL;
                }
                config->prefix = strdup(arg);
                if (!config->prefix)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_SUFFIX:
                if (config->suffix)
                {
                        free(config->suffix);
                        config->suffix = NULL;
                }
                config->suffix = strdup(arg);
                if (!config->suffix)
                {
                        err = ENOMEM;
                }
                break;
/*
        case ARGP_KEY_SUCCESS:
        case ARGP_KEY_ERROR:
        case ARGP_KEY_END:
                break;
*/
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
                LOG("Error parsing MemTotal: %d\n", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, "MemFree", &free);
        if (err)
        {
                LOG("Error parsing MemFree: %d\n", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, "Buffers", buffers);
        if (err)
        {
                LOG("Error parsing Buffers: %d\n", err);
                return err;
        }

        err = parse_meminfo_field(meminfo, "Cached", cached);
        if (err)
        {
                LOG("Error parsing Cached: %d\n", err);
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
