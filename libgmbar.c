#include "libgmbar.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
 * @param   width   Width of the section
 * @param   color   Color of the section.
 * @return  Zero on success, non-zero on failure.
 */
int
gmbar_add_section(gmbar* bar, unsigned int width, char* color)
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
                        section->width = width;
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
 * Adds a new section to the bar.
 *
 * On successful execution, takes ownership of the @color pointer.
 *
 * @param   width   Width of the section
 * @param   color   Color of the section.
 * @return  Zero on success, non-zero on failure.
 */
int
gmbar_add_section_by_value(gmbar* bar, unsigned int total, unsigned int value, char* color)
{
        unsigned int width = (unsigned int) bar->size.width * ((double)value / total);
        return gmbar_add_section(bar, width, color);
}

/**
 *
 *
 * @param   bar   Bar to textualize
 * @param   nl    If non-zero, newline is added to the end of the string
 * @return A newly allocated buffer or NULL if allocation failed.  Caller
 * is responsible for freeing the returned buffer.
 */
char*
gmbar_format(gmbar* bar, unsigned int nl)
{
        /* width of the bar without margins */
        int bar_width = bar->size.width - bar->margin.left - bar->margin.right;
        /* height of the sections */
        unsigned int section_height = bar->size.height
                - bar->margin.top - bar->margin.bottom
                - bar->padding.top - bar->padding.bottom;
        /* number of pixels from the start of the first section to the
         * right margin */
        int width = bar_width - bar->padding.left + bar->margin.right;

        int size = 1024;
        int nchars = 0;
        int written = 0;

        unsigned int i = 0;
        gmsection* section = NULL;

        char* buf = (char*) malloc(size);
        if (buf)
        {
                /* draw the outline and put position to start of the first section */
                written = snprintf(buf, size, "^ib(1)^p(%u)^fg(%s)^ro(%ux%u)^p(%d)",
                                   bar->margin.left,
                                   bar->color.fg,
                                   bar_width,
                                   bar->size.height - bar->margin.top - bar->margin.bottom,
                                   -bar_width + bar->padding.left);
                nchars += written;
                size -= written;

                /* draw the sections */
                for (i = 0, section = NULL;
                     i < bar->nsections && (section = bar->sections[i]) && size > 0;
                     i++, width -= section->width, nchars += written, size -= written)
                {
                        written = snprintf(&buf[nchars], size, "^fg(%s)^r(%ux%u)",
                                           section->color,
                                           section->width,
                                           section_height);
                }

                if (size > 0)
                {
                        /* move position to right margin */
                        written = snprintf(&buf[nchars], size, "^p(%d)", width);
                        nchars += written;
                        size -= written;
                }

                if (size <= 0)
                {
                        free(buf);
                        buf = NULL;
                }
        }

        return buf;
}
