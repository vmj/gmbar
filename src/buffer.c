#include "buffer.h"
#include <stdlib.h>
#include <string.h>

/**
 * Creates a new buffer.
 *
 * @return  A newly allocated buffer or NULL if there was not enought memory
 *          to allocate one.
 */
buffer*
buffer_new()
{
        buffer* buf = (buffer*) malloc(sizeof(buffer));
        if (buf)
        {
                memset(buf, 0, sizeof(buffer));
        }
        return buf;
}

/**
 *
 */
void
buffer_free(buffer* buf)
{
        if (buf)
        {
                if (buf->buf)
                {
                        free(buf->buf);
                }
                free(buf);
        }
}
