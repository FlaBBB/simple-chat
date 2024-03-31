#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

typedef struct
{
    char **buff;
    unsigned int start;
    unsigned int end;
    unsigned int capacity;
    int isFull;
} msg_layout;

int term_w, term_h;
msg_layout msg_l;

void message_init();
void resize();
void add_message(char *message, size_t len_msg);
void render_ui(char *msg);

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
    msg_l.buff = (char **)malloc(sizeof(char *) * (term_h - 1));
    msg_l.start = 0;
    msg_l.end = 0;
    msg_l.capacity = (term_h - 1);
    msg_l.isFull = 0;

    clear_screen();
    printf("\033[%d;1HInput your message> ", msg_l.capacity + 1);
}

void reorganize()
{
    char **temp_buff = (char **)malloc(sizeof(char *) * msg_l.capacity);
    memcpy(temp_buff, msg_l.buff, sizeof(char *) * msg_l.capacity);
    int i = 0, j = msg_l.start;
    do
    {
        free(msg_l.buff[i]);
        size_t lbuff = strlen(temp_buff[j]);
        msg_l.buff[i] = (char *)malloc(sizeof(char) * lbuff);
        memcpy(msg_l.buff[i], temp_buff[j], sizeof(char) * lbuff);
        j = (j + 1) % msg_l.capacity;
    } while (j != msg_l.end);
}

void resize()
{
    clear_screen();

    // reorganize if the message not sorted perfectly
    if (msg_l.end < msg_l.start)
        reorganize();
    if (term_h - 1 > msg_l.capacity)
        msg_l.isFull = 0;

    msg_l.capacity = (term_h - 1);
    msg_l.buff = (char **)realloc(msg_l.buff, sizeof(char *) * msg_l.capacity);
}

void add_message(char *message, size_t len_msg)
{
    // check if the message layout is full
    // if it is full, remove the oldest message
    if (msg_l.isFull)
    {
        msg_l.start = (msg_l.start + 1) % msg_l.capacity;
        free(msg_l.buff[msg_l.end]);
    }

    msg_l.buff[msg_l.end] = (char *)malloc(sizeof(char) * (len_msg + 1));
    memcpy(msg_l.buff[msg_l.end], message, sizeof(char) * len_msg);
    msg_l.buff[msg_l.end][len_msg] = '\0';

    if (!msg_l.isFull && msg_l.end == msg_l.capacity - 1)
        msg_l.isFull = 1;

    msg_l.end = (msg_l.end + 1) % msg_l.capacity;
}

void print_all_message()
{
    int i = msg_l.start, e = msg_l.end;
    do
    {
        printf("%s\033[K\n", msg_l.buff[i]);
        i = (i + 1) % msg_l.capacity;
    } while (i != e);
}

void render_ui(char *msg)
{
    int _term_w = 0, _term_h = 0;
    getWindowsSize(&_term_h, &_term_w);

    if (_term_h != term_h || _term_w != term_w)
    {
        term_h = _term_h;
        term_w = _term_w;
        resize();
    }

    setvbuf(stdout, NULL, _IOFBF, 2 * term_w * term_h); // set buffer to 2 times of terminal size
    printf("\033[%d;1H\033[K", msg_l.capacity);

    printf("\033[1;1H");
    print_all_message();

    printf("\033[%d;1HInput your message> %s", msg_l.capacity + 1, msg);
    fflush(stdout);
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
}