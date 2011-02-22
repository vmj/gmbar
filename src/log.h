#ifndef LOG_H
#define LOG_H

int log_open(char* filename);

void log_error(char* frmt, int err);

#endif //LOG_H
