#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "readfile.h"
#include "log.h"


char*
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
                log_error("Error opening file: %d\n", err);
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
                                log_error("Error allocating space for file contents: %d", err);
                                break;
                        }
                        buf = tmp;
                }
                bytes = read(fd, buf + bufsize, bufmax - bufsize);
                switch (bytes)
                {
                case -1:
                        err = errno;
                        log_error("Error reading file: %d\n", err);
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
                        log_error("Error closing file: %d\n", err);
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

