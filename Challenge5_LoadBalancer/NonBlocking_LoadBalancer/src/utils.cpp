#include "../include/utils.h"

bool set_non_blocking(int fd) {
    if (fd < 0) 
        return false;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) 
        return false;

    flags = flags | O_NONBLOCK;
    return (fcntl(fd, F_SETFL, flags) == 0);
}