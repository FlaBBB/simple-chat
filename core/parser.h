#include "io_socket.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int32_t receive_message(int fd, char *buff, size_t buff_size)
{
    errno = 0;
    size_t len = 0;
    int32_t err = read_full(fd, (char *)&len, 2);
    if (err)
    {
        if (errno != 0)
        {
            perror("read() error");
        }
        return err;
    }

    if (len > buff_size)
    {
        perror("message too long");
        return -1;
    }

    err = read_full(fd, buff, len);
    if (err)
    {
        perror("read() error");
        return err;
    }

    buff[2 + len] = '\0';
    return 0;
}

int32_t  send_message(int fd, char *buff, size_t length_buff)
{
    errno = 0;
    char wbuff[2 + length_buff];
    memcpy(wbuff, &length_buff, 2);
    memcpy(&wbuff[2], buff, length_buff);
    int32_t err = write_all(fd, wbuff, length_buff + 2);
    if (err)
    {
        if (errno == 0)
        {
            perror("EOF");
        }
        else
        {
            perror("write() error");
        }
        return err;
    }
    return 0;
}