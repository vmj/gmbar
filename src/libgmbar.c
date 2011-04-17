#include "libgmbar.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/**
 * Creates a new gmbar.
 *
 * @return  A newly allocated gmbar or NULL if there was not enought memory
 *          to allocate one.
 */
gmbar*
gmbar_new()
{
        gmbar* bar = (gmbar*) malloc(sizeof(gmbar));
        if (bar)
        {
                memset(bar, 0, sizeof(gmbar));
        }
        return bar;
}

/**
 * Creates a new gmbar.
 *
 * @param   width    Width
 * @param   height   Height
 * @param   fg       Foreground color
 * @param   bg       Background color
 * @return  A newly allocated gmbar or NULL if there was not enought memory
 *          to allocate one.
 */
gmbar*
gmbar_new_with_defaults(unsigned int width, unsigned int height, char* fg, char* bg)
{
        gmbar* bar = gmbar_new();
        if (bar)
        {
                bar->size.width  = width;
                bar->size.height = height;
                bar->color.fg = strdup(fg);
                bar->color.bg = strdup(bg);
                if (!bar->color.fg || !bar->color.bg)
                {
                        gmbar_free(bar);
                        bar = NULL;
                }
        }
        return bar;
}

/**
 *
 */
void
gmbar_free(gmbar* bar)
{
        int i = 0;
        if (bar)
        {
                for (i = bar->nsections - 1; i >= 0; i--)
                {
                        gmsection* section = bar->sections[i];
                        if (section)
                        {
                                if (section->color)
                                {
                                        free(section->color);
                                }
                                free(section);
                        }
                }
                if (bar->sections)
                {
                        free(bar->sections);
                }
                if (bar->color.fg)
                {
                        free(bar->color.fg);
                }
                if (bar->color.bg)
                {
                        free(bar->color.bg);
                }
                free(bar);
        }
}

/**
 * Adds a new section to the bar.
 *
 * On successful execution, takes ownership of the @color pointer.
 *
 * @param   color   Color of the section.
 * @return  Zero on success, non-zero on failure.
 */
int
gmbar_add_section(gmbar* bar, char* color)
{
        gmsection* section = (gmsection*) malloc(sizeof(gmsection));
        if (section)
        {
                gmsection** sections = (gmsection**) malloc(sizeof(gmsection*) * (bar->nsections + 1));
                if (sections)
                {
                        if (bar->sections)
                        {
                                memcpy(sections, bar->sections, sizeof(gmsection*) * bar->nsections);
                                free(bar->sections);
                        }
                        bar->sections = sections;
                        bar->sections[bar->nsections] = section;
                        section->bar   = bar;
                        section->width = 0;
                        section->color = color;
                        bar->nsections++;
                }
                else
                {
                        free(section);
                        section = NULL;
                }
        }

        return section ? 0 : -1;
}

/**
 * Adds new sections to the bar.
 *
 * @param   nsections   Number of sections
 * @param   ...         Colors for the sections (const char*)
 * @return  Zero on success, non-zero on failure
 */
int
gmbar_add_sections(gmbar* bar, unsigned int nsections, ...)
{
        int err = 0;
        unsigned int i = 0;
        char* color = NULL;
        va_list argv;
        va_start(argv, nsections);
        for ( ; i < nsections; i++ )
        {
                color = va_arg(argv, char*);
                color = strdup(color);
                if (!color)
                {
                        err = -1;
                        break;
                }
                err = gmbar_add_section(bar, color);
                if (err)
                {
                        break;
                }
        }
        va_end(argv);
        return err;
}

/**
 * Sets a section width given a value.
 *
 * @param   total   Total value
 * @param   value   Value
 */
void
gmbar_set_section_width(gmsection* section, unsigned int total, unsigned int value)
{
        gmbar* bar = (gmbar*)section->bar;
        unsigned int inner_width = (unsigned int) (bar->size.width
                                                   - bar->margin.left - bar->margin.right
                                                   - bar->padding.left - bar->padding.right);
        section->width = inner_width * ((double)value / total);
}

/**
 *
 *
 * @param   bar   Bar to textualize
 * @param   nl    If non-zero, newline is added to the end of the string
 * @param   data  On return, a newly allocated buffer or NULL if
 *                allocation failed.  Caller is responsible for freeing
 *                the returned buffer.
 * @return  Zero on success, non-zero on failure.
 */
int
gmbar_format(gmbar* bar, unsigned int nl, char** buf, int* len, int* max)
{
        const int orig_len = *len;
        int err = 0;
        char* tmp = NULL;
        /* width of the bar without margins */
        int bar_width = bar->size.width - bar->margin.left - bar->margin.right;
        /* height of the sections */
        unsigned int section_height = bar->size.height
                - bar->margin.top - bar->margin.bottom
                - bar->padding.top - bar->padding.bottom;
        /* number of pixels from the start of the first section to the
         * right margin */
        int width = bar_width - bar->padding.left + bar->margin.right;
        int current_segment_width = 0;
        int current_gap_width = 0;

        unsigned int i = 0;
        gmsection* section = NULL;

        do
        {
                if (*len >= *max)
                {
                        tmp = realloc(*buf, *max + 1024);
                        if (!tmp)
                        {
                                err = errno;
                                break;
                        }
                        *max += 1024;
                        *buf = tmp;
                        *len = orig_len;
                }

                /* draw the outline and put position to start of the first section */
                if ( *max - *len > 0 )
                {
                        *len += snprintf(*buf + *len, *max - *len, "^ib(1)^p(%u)^fg(%s)^ro(%ux%u)^p(%d)",
                                         bar->margin.left,
                                         bar->color.fg,
                                         bar_width,
                                         bar->size.height - bar->margin.top - bar->margin.bottom,
                                         -bar_width + bar->padding.left);
                }

                /* draw the sections */
                for (i = 0, section = NULL, current_segment_width = bar->segment_width, current_gap_width = bar->segment_gap;
                     i < bar->nsections && (section = bar->sections[i]) && *max - *len > 0;
                     i++, width -= section->width)
                {
                        if (section->width == 0)
                        {
                                // nothing to do
                        }
                        else if (strcmp(section->color, "none") == 0)
                        {
                                *len += snprintf(*buf + *len, *max - *len, "^p(%u)", section->width);
                        }
                        else
                        {
                                if (bar->segment_width == 0 || bar->segment_gap == 0)
                                {
                                        *len += snprintf(*buf + *len, *max - *len, "^fg(%s)^r(%ux%u)",
                                                         section->color,
                                                         section->width,
                                                         section_height);
                                }
                                else
                                {
                                        int section_width = section->width;
                                        *len += snprintf(*buf + *len, *max - *len, "^fg(%s)", section->color);
                                        while (section_width > 0 && *max - *len > 0)
                                        {
                                                if (current_segment_width > 0)
                                                {
                                                        if (current_segment_width >= section_width)
                                                        {
                                                                *len += snprintf(*buf + *len, *max - *len,
                                                                                 "^r(%ux%u)",
                                                                                 section_width,
                                                                                 section_height);
                                                                current_segment_width -= section_width;
                                                                section_width = 0;
                                                                if (current_segment_width == 0)
                                                                {
                                                                        current_gap_width = bar->segment_gap;
                                                                }

                                                        }
                                                        else
                                                        {
                                                                *len += snprintf(*buf + *len, *max - *len,
                                                                                 "^r(%ux%u)",
                                                                                 current_segment_width,
                                                                                 section_height);
                                                                section_width -= current_segment_width;
                                                                current_segment_width = 0;
                                                                current_gap_width = bar->segment_gap;
                                                        }
                                                }
                                                else
                                                {
                                                        if (current_gap_width >= section_width)
                                                        {
                                                                *len += snprintf(*buf + *len, *max - *len,
                                                                                 "^p(%u)",
                                                                                 section_width);
                                                                current_gap_width -= section_width;
                                                                section_width = 0;
                                                                if (current_gap_width == 0)
                                                                {
                                                                        current_segment_width = bar->segment_width;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                *len += snprintf(*buf + *len, *max - *len,
                                                                                 "^p(%u)",
                                                                                 current_gap_width);
                                                                section_width -= current_gap_width;
                                                                current_gap_width = 0;
                                                                current_segment_width = bar->segment_width;
                                                        }
                                                }
                                        }
                                }
                        }
                }

                if (*max - *len > 0)
                {
                        /* move position to right margin */
                        *len += snprintf(*buf + *len, *max - *len, "^p(%d)", width);
                }

                if (*max - *len > 0)
                {
                        /* everything fits */
                        break;
                }
        }
        while(1);

        return err;
}
