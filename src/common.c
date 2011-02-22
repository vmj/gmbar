#include <stdlib.h>
#include <string.h>

#include "common.h"


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
          "Background color of the bar (not used at the moment)"},
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
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->margin.top = int_value;
                break;
        case OPTION_MARGIN_RIGHT:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->margin.right = int_value;
                break;
        case OPTION_MARGIN_BOTTOM:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->margin.bottom = int_value;
                break;
        case OPTION_MARGIN_LEFT:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->margin.left = int_value;
                break;
        case OPTION_PADDING_TOP:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->padding.top = int_value;
                break;
        case OPTION_PADDING_RIGHT:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->padding.right = int_value;
                break;
        case OPTION_PADDING_BOTTOM:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->padding.bottom = int_value;
                break;
        case OPTION_PADDING_LEFT:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->padding.left = int_value;
                break;
        case OPTION_WIDTH:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->size.width = int_value;
                break;
        case OPTION_HEIGHT:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->size.height = int_value;
                break;
        case OPTION_FOREGROUND_COLOR:
                err = parse_option_arg_string(arg, &config->bar->color.fg);
                break;
        case OPTION_BACKGROUND_COLOR:
                err = parse_option_arg_string(arg, &config->bar->color.bg);
                break;
        case OPTION_MARGIN:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->margin.top = int_value;
                config->bar->margin.right = int_value;
                config->bar->margin.bottom = int_value;
                config->bar->margin.left = int_value;
                break;
        case OPTION_PADDING:
                int_value = parse_option_arg_unsigned_int(arg);
                config->bar->padding.top = int_value;
                config->bar->padding.right = int_value;
                config->bar->padding.bottom = int_value;
                config->bar->padding.left = int_value;
                break;
        case OPTION_UPDATE_INTERVAL:
                int_value = parse_option_arg_unsigned_int(arg);
                config->interval = int_value;
                break;
        case OPTION_LOG_FILE:
                err = parse_option_arg_string(arg, &config->log_file);
                break;
        case OPTION_PREFIX:
                err = parse_option_arg_string(arg, &config->prefix);
                break;
        case OPTION_SUFFIX:
                err = parse_option_arg_string(arg, &config->suffix);
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
 * @return  Parsed value.
 */
unsigned int
parse_option_arg_unsigned_int(char* arg)
{
        unsigned int value = 0;
        while (!isdigit(*arg))
                arg++;
        while(isdigit(*arg))
                value = value * 10 + (*arg++ - '0');
        return value;
}

/**
 * Copy @arg to @str, taking care of memory management.
 *
 * @param   arg   Argument
 * @param   str   On return, points to copy of arg
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
