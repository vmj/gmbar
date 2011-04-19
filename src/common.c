#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "log.h"


/* Common argp parser function */
static error_t handle_common_option(int key,
                                    char* arg,
                                    struct argp_state *state);

/* Common argp option keys */
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
        OPTION_UPDATE_INTERVAL = 'i',
        OPTION_LOG_FILE = 'L',
        OPTION_PREFIX = 'P',
        OPTION_SUFFIX = 'S',
        OPTION_SEGMENT_WIDTH = 's',
        OPTION_SEGMENT_GAP = 'g',
        OPTION_GRANULARITY = 'G',
        OPTION_ROUNDING = 'R',
};

/* Common options */
static const struct argp_option common_options[] = {
        { "width",      OPTION_WIDTH,              "WIDTH",     0,
          "Width of the bar in pixels, including margins"       },
        { "height",     OPTION_HEIGHT,             "HEIGHT",    0,
          "Height of the bar in pixels, including margins"      },
        { "fg",         OPTION_FOREGROUND_COLOR,   "COLOR",     0,
          "Foreground color of the bar (the outline color)"     },
        { "bg",         OPTION_BACKGROUND_COLOR,   "COLOR",     0,
          "Background color of the whole bar"                   },
        { "margin",     OPTION_MARGIN,             "MARGIN",    0,
          "Margin size in pixels, all four sides"               },
        { "padding",    OPTION_PADDING,            "PADDING",   0,
          "Padding size in pizels, all four sides"              },
        { "interval",   OPTION_UPDATE_INTERVAL,    "SECONDS",   0,
          "Polling intetrval in seconds (zero disables polling)"},
        { "logfile",    OPTION_LOG_FILE,           "LOGFILE",   0,
          "Log debug messages to file"                          },
        { "prefix",     OPTION_PREFIX,             "PREFIX",    0,
          "Prefix to print before the bar"                      },
        { "suffix",     OPTION_SUFFIX,             "SUFFIX",    0,
          "Suffix to print after the bar"                       },
        { "segment",    OPTION_SEGMENT_WIDTH,      "WIDTH",     0,
          "Segment width in pixels"                             },
        { "gap",        OPTION_SEGMENT_GAP,        "WIDTH",     0,
          "Gap between segments, in pixels"                     },
        { "granularity", OPTION_GRANULARITY,       "WIDTH",     0,
          "Granularity of sections, in pixels"                  },
        { "rounding",   OPTION_ROUNDING,           "WIDTH",     0,
          "Rounding point for sections (default: half of granularity)" },
        { 0 }
};

/* Common argp parsers */
static const struct argp common_argp[] = {
        { common_options, handle_common_option, NULL, NULL },
        { 0 }
};


/* Child argp parsers */
const struct argp_child common_argp_child[] = {
        { common_argp, 0, "Common options", 0 },
        { 0 }
};


/**
 * Argp parser function.
 */
static error_t
handle_common_option(int key, char* arg, struct argp_state *state)
{
        error_t err = 0;
        unsigned int int_value = 0;
        common_arguments* config = (common_arguments*) state->input;

        switch (key)
        {
        case OPTION_MARGIN_TOP:
                err = parse_option_arg_unsigned_char(arg, &config->bar->margin.top);
                break;
        case OPTION_MARGIN_RIGHT:
                err = parse_option_arg_unsigned_char(arg, &config->bar->margin.right);
                break;
        case OPTION_MARGIN_BOTTOM:
                err = parse_option_arg_unsigned_char(arg, &config->bar->margin.bottom);
                break;
        case OPTION_MARGIN_LEFT:
                err = parse_option_arg_unsigned_char(arg, &config->bar->margin.left);
                break;
        case OPTION_PADDING_TOP:
                err = parse_option_arg_unsigned_char(arg, &config->bar->padding.top);
                break;
        case OPTION_PADDING_RIGHT:
                err = parse_option_arg_unsigned_char(arg, &config->bar->padding.right);
                break;
        case OPTION_PADDING_BOTTOM:
                err = parse_option_arg_unsigned_char(arg, &config->bar->padding.bottom);
                break;
        case OPTION_PADDING_LEFT:
                err = parse_option_arg_unsigned_char(arg, &config->bar->padding.left);
                break;
        case OPTION_WIDTH:
                err = parse_option_arg_unsigned_int(arg, &config->bar->size.width);
                break;
        case OPTION_HEIGHT:
                err = parse_option_arg_unsigned_int(arg, &config->bar->size.height);
                break;
        case OPTION_FOREGROUND_COLOR:
                err = parse_option_arg_string(arg, &config->bar->color.fg);
                break;
        case OPTION_BACKGROUND_COLOR:
                err = parse_option_arg_string(arg, &config->bar->color.bg);
                break;
        case OPTION_MARGIN:
                err = parse_option_arg_unsigned_int(arg, &int_value);
                config->bar->margin.top = int_value;
                config->bar->margin.right = int_value;
                config->bar->margin.bottom = int_value;
                config->bar->margin.left = int_value;
                break;
        case OPTION_PADDING:
                err = parse_option_arg_unsigned_int(arg, &int_value);
                config->bar->padding.top = int_value;
                config->bar->padding.right = int_value;
                config->bar->padding.bottom = int_value;
                config->bar->padding.left = int_value;
                break;
        case OPTION_UPDATE_INTERVAL:
                err = parse_option_arg_unsigned_int(arg, &config->interval);
                break;
        case OPTION_LOG_FILE:
                err = log_open(arg);
                if (err)
                {
                        err = ENOMEM;
                }
                break;
        case OPTION_PREFIX:
                err = parse_option_arg_string(arg, &config->prefix);
                break;
        case OPTION_SUFFIX:
                err = parse_option_arg_string(arg, &config->suffix);
                break;
        case OPTION_SEGMENT_WIDTH:
                err = parse_option_arg_unsigned_int(arg, &config->bar->segment_width);
                break;
        case OPTION_SEGMENT_GAP:
                err = parse_option_arg_unsigned_int(arg, &config->bar->segment_gap);
                break;
        case OPTION_GRANULARITY:
                err = parse_option_arg_unsigned_int(arg, &config->bar->granularity);
                break;
        case OPTION_ROUNDING:
                err = parse_option_arg_double(arg, &config->bar->rounding);
                break;

        default:
                err = ARGP_ERR_UNKNOWN;
                break;
        }

        return err;
}


/**
 * Parse argument as unsigned integer.
 *
 * @param   arg   Argument
 * @param   val   On return, points to the parsed value
 * @return  Zero (no error detection).
 */
int
parse_option_arg_unsigned_int(char* arg, unsigned int* value)
{
        *value = 0;
        while (!isdigit(*arg))
                arg++;
        while(isdigit(*arg))
                *value = *value * 10 + (*arg++ - '0');
        return 0;
}

/**
 * Parse argument as double
 *
 * @param   arg   Argument
 * @param   val   On return, points to the parsed value
 * @return  Zero (no error detection).
 */
int
parse_option_arg_double(char* arg, double* value)
{
        *value = atof(arg);
        return 0;
}

/**
 * Parse argument as unsigned char.
 *
 * @param   arg   Argument
 * @param   val   On return, points to the parsed value
 * @return  Zero (no error detection).
 */
int
parse_option_arg_unsigned_char(char* arg, unsigned char* value)
{
        *value = 0;
        while (!isdigit(*arg))
                arg++;
        while(isdigit(*arg))
                *value = *value * 10 + (*arg++ - '0');
        return 0;
}

/**
 * Copy @arg to @str, taking care of memory management.
 *
 * @param   arg   Argument
 * @param   str   On return, points to copy of arg
 * @return  ENOMEM (error_t) if memory allocation failed, zero otherwise.
 */
int
parse_option_arg_string(char* arg, char** str)
{
        error_t err = 0;
        if (*str)
        {
                free(*str);
                *str = NULL;
        }
        *str = strdup(arg);
        if (!*str)
        {
                err = ENOMEM;
        }
        return err;
}

/**
 * Find @str in @mem.
 *
 * If @str is zero length or NULL, @mem is returned.
 *
 * @param   mem    Memory buffer from which to search
 * @param   str    Zero terminated string to search
 * @param   size   Size of the mem buffer in bytes
 * @return  
 */
const char*
memstr(const char* mem, const char* str, unsigned int size)
{
        const size_t len = str ? strlen(str) : 0;

        if (len == 0)
                return mem;

        while (size >= len)
        {
                if (memcmp(mem, str, len) == 0)
                        return mem;
                mem++;
                size--;
        }

        return NULL;
}

/**
 *
 */
int
print_bar(common_arguments* args)
{
        static char* buf = NULL;
        static int len = 0;
        static int max = 0;
        int err = 0;

        len = 0;
        err = gmbar_format(args->bar, 0, &buf, &len, &max);
        if (!err)
        {
                if (args->prefix && args->suffix)
                {
                        printf("%s%s%s\n", args->prefix, buf, args->suffix);
                }
                else if (args->prefix)
                {
                        printf("%s%s\n", args->prefix, buf);
                }
                else if (args->suffix)
                {
                        printf("%s%s\n", buf, args->suffix);
                }
                else
                {
                        printf("%s\n", buf);
                }
        }
        else
        {
                printf("^fg(red)^bg(black)%d^bg()^fg()\n", err);
        }
        fflush(stdout);
        return err;
}

