#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer buffer;
struct buffer {
        char* buf;
        unsigned int len;
        unsigned int max;
};

buffer*   buffer_new   ();
void      buffer_free  (buffer* buf);

#endif //BUFFER_H
