#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "../tools/string_helpers.h"
#include <sys/param.h>

typedef struct
{
    char **buff;
    unsigned int start;
    unsigned int end;
    unsigned int size;
    unsigned int capacity;
} msg_layout;

int term_w, term_h;
msg_layout msg_l;

void getWindowsSize(int *row, int *col)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        perror("ioctl");
        return;
    }
    *row = ws.ws_row;
    *col = ws.ws_col;
}

void clear_screen()
{
    printf("\033[2J");
    printf("\033[1;1H");
}

void message_init()
{
    getWindowsSize(&term_h, &term_w);
    msg_l.buff = malloc(sizeof(*msg_l.buff) * (term_h - 1));
    if (msg_l.buff == NULL)
    {
        printf("memory allocation failed");
        exit(1);
    }

    msg_l.capacity = (term_h - 1);
    msg_l.size = 0;
    msg_l.start = 0;
    msg_l.end = 0;

    clear_screen();
    printf("\033[%d;1HInput your message> ", msg_l.capacity + 1);
}

void checkNResize()
{
    int _term_w = 0, _term_h = 0;
    getWindowsSize(&_term_h, &_term_w);

    if (!(_term_h != term_h || _term_w != term_w))
    {
        return;
    }
    term_h = _term_h;
    term_w = _term_w;

    clear_screen();

    // make the start ~> end is less or equals with (term_h - 1)
    int new_capacity = term_h - 1;
    if (new_capacity < msg_l.size)
    {
        msg_l.start = (msg_l.start + msg_l.size - new_capacity) % msg_l.capacity;
        msg_l.size = new_capacity;
    }

    char **new_buff = malloc(sizeof(char *) * new_capacity);

    unsigned int i = 0, j = msg_l.start, e = msg_l.end;
    do
    {
        new_buff[i++] = msg_l.buff[j];

        j = (j + 1) % msg_l.capacity;
    } while (j != e);

    free(msg_l.buff);

    msg_l.buff = new_buff;
    msg_l.capacity = new_capacity;
    msg_l.start = 0;
    msg_l.end = i % msg_l.capacity;
}

void add_message(char *message, size_t len_msg)
{
    // check if the message layout is full
    // if it is full, remove the oldest message
    if (msg_l.size == msg_l.capacity)
    {
        free(msg_l.buff[msg_l.start]);
        msg_l.start = ((unsigned int)(msg_l.start + 1)) % msg_l.capacity;
    }

    msg_l.buff[msg_l.end] = malloc((len_msg + 1) * sizeof(*msg_l.buff[msg_l.end]));
    if (msg_l.buff[msg_l.end] == NULL)
    {
        printf("memory allocation failed");
        exit(1);
    }

    memcpy(msg_l.buff[msg_l.end], message, len_msg);
    msg_l.buff[msg_l.end][len_msg] = '\0';

    if (msg_l.size < msg_l.capacity) {
        msg_l.size++;
    }

    msg_l.end = ((unsigned int)(msg_l.end + 1)) % msg_l.capacity;
}

void print_all_message()
{
    unsigned int i = msg_l.start, e = msg_l.end;
    do
    {
        printf("%s\033[K\n", msg_l.buff[i]);
        i = (i + 1) % msg_l.capacity;
    } while (i != e);
}

void render_ui(char *msg)
{
    setvbuf(stdout, NULL, _IOFBF, 2 * term_w * term_h); // set buffer to 2 times of terminal size
    printf("\033[%d;1H\033[K", msg_l.capacity);

    printf("\033[1;1H");
    print_all_message();

    printf("\033[%d;1HInput your message> %s", msg_l.capacity + 1, msg);
    fflush(stdout);
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
}