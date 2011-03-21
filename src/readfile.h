#ifndef READFILE_H
#define READFILE_H

/**
 *
 * @param   filepath
 * @param   buf
 * @param   size
 * @param   max
 * @return
 */
int   readfile   (const char* filepath,
                  char** buf,
                  unsigned int* size,
                  unsigned int* max);

#endif //READFILE_H
