#include <unistd.h>
#include <assert.h>
#include <stdint.h>

int32_t read_full (int fd, char *buff, size_t size) {
    while (size > 0)
    {
        ssize_t rv = read(fd, buff, size);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= size);
        buff += rv;
        size -= rv;
    }
    return 0;
}

int32_t write_all (int fd, char *buff, size_t size) {
    while (size > 0)
    {
        ssize_t rv = write(fd, buff, size);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= size);
        buff += rv;
        size -= rv;
    }
    return 0;
}