#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "readfile.h"
#include "log.h"


int
readfile(const char* filepath, char** data, unsigned int* size, unsigned int* max)
{
        int err = 0;
        int fd = 0;
        ssize_t bytes = -1;
        char* tmp = NULL;

        fd = open(filepath, O_RDONLY);
        if (fd == -1)
        {
                err = errno;
                log_error("Error opening file: %d\n", err);
        }

        while (!err && bytes != 0)
        {

                if (*size == *max)
                {
                        *max += 1024;
                        tmp = realloc(*data, *max);
                        if (!tmp)
                        {
                                *max -= 1024;
                                err = errno;
                                log_error("Error allocating space for file contents: %d\n", err);
                                break;
                        }
                        *data = tmp;
                }
                bytes = read(fd, *data + *size, *max - *size);
                switch (bytes)
                {
                case -1:
                        err = errno;
                        log_error("Error reading file: %d\n", err);
                        break;
                default:
                        *size += bytes;
                        break;
                }
        }

        if (fd != -1)
        {
                fd = close(fd);
                if (fd == -1)
                {
                        if (!err) err = errno;
                        log_error("Error closing file: %d\n", errno);
                }
        }

        return err;
}

