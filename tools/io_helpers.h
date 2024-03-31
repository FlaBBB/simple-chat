#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

// Function to set terminal attributes for non-canonical mode
void set_terminal_mode()
{
    struct termios t;

    // Get the current terminal attributes
    tcgetattr(STDIN_FILENO, &t);

    // Set the terminal to non-canonical mode
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VTIME] = 0;
    t.c_cc[VMIN] = 1;

    // Set the new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Function to restore terminal attributes
void restore_terminal_mode()
{
    // Get the current terminal attributes
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);

    // Restore canonical mode and echo
    t.c_lflag |= ICANON | ECHO;

    // Set the new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// get input with non-canonial terminal
size_t get_input_noncanon(char *buff, size_t buff_size, int *hold_flag)
{
    set_terminal_mode();

    size_t lbuff = 0;
    int pos = 0;
    char c;
    int flag;
    int n, m;
    while (1)
    {
        while (*hold_flag == 1)
        {
        }

        // Read a single character from stdin
        if (read(STDIN_FILENO, &c, 1) == -1)
        {
            perror("read");
            exit(1);
        }

        // Check for special keys
        if (c == 27) // Escape sequence (arrow keys)
        {
            if (read(STDIN_FILENO, &c, 1) == -1)
            {
                perror("read");
                exit(1);
            }
            if (c == 91) // '['
            {
                flag = 0;
                n = 0;
                m = 0;
                do
                {
                    if (read(STDIN_FILENO, &c, 1) == -1)
                    {
                        perror("read");
                        exit(1);
                    }

                    if (c == ';')
                        flag++;
                    if (c >= '0' && c <= '9')
                    {
                        if (flag == 0)
                        {
                            n *= 10;
                            n += c - '0';
                        }
                        else if (flag == 1)
                        {
                            m *= 10;
                            m += c - '0';
                        }
                    }

                } while ((c >= '0' && c <= '9') || c == ';');

                switch (c)
                {
                case 68: // Left arrow
                    if (n == 1 && m == 5)
                    {
                        int temp = pos;
                        flag = 0;
                        while (temp > 0 && (buff[temp - 1] != ' ' || flag == 0))
                        {
                            if (buff[temp - 1] != ' ')
                                flag = 1;
                            temp--;
                        }
                        if (temp != pos)
                            printf("\033[%dD", pos - temp);
                        pos = temp;
                    }
                    else if (pos > 0)
                    {
                        printf("\b");
                        pos--;
                    }
                    break;
                case 67: // Right arrow
                    if (n == 1 && m == 5)
                    {

                        int temp2 = pos;
                        flag = 0;
                        while (temp2 < lbuff && (buff[temp2] != ' ' || flag == 0))
                        {
                            if (buff[temp2] != ' ')
                                flag = 1;
                            temp2++;
                        }
                        if (temp2 != pos)
                            printf("\033[%dC", temp2 - pos);
                        pos = temp2;
                    }
                    else if (pos < lbuff)
                    {
                        putchar(buff[pos]);
                        pos++;
                    }
                    break;
                case 51: // Delete key
                    if (pos < lbuff)
                    {
                        for (int i = pos; i < lbuff; i++)
                        {
                            buff[i] = buff[i + 1];
                        }
                        lbuff--;
                        printf("\033[K");
                        for (int i = pos; i < lbuff; i++)
                        {
                            putchar(buff[i]);
                        }
                        printf("\033[%ldD", lbuff - pos);
                    }
                    break;
                case 72: // Home key
                    if (pos <= 0)
                        goto FLUSH;
                    printf("\033[%dD", pos);
                    pos = 0;
                    break;
                case 70: // End key
                    if (pos >= lbuff)
                        goto FLUSH;
                    printf("\033[%ldC", lbuff - pos);
                    pos = lbuff;
                    break;
                default:
                    break;
                }
            }
        }
        else if (c == 127) // Backspace key
        {
            if (pos > 0)
            {
                for (int i = pos - 1; i < lbuff; i++)
                {
                    buff[i] = buff[i + 1];
                }
                lbuff--;
                pos--;
                printf("\b\033[K");
                for (int i = pos; i < lbuff; i++)
                {
                    putchar(buff[i]);
                }
                if (lbuff - pos > 0)
                    printf("\033[%ldD", lbuff - pos);
            }
        }
        else if (c == 23) // CTRL + BACKSPACE
        {
            int temp = pos;
            flag = 0;
            while (temp > 0 && (buff[temp - 1] != ' ' || flag == 0))
            {
                if (buff[temp - 1] != ' ')
                    flag = 1;
                buff[temp - 1] = '\0';
                temp--;
            }
            if (temp != pos)
            {
                printf("\033[%dD\033[K", pos - temp);
                for (int i = pos; i < lbuff; i++)
                {
                    putchar(buff[i]);
                    buff[i + pos - temp] = buff[i];
                    buff[i] = '\0';
                }
                lbuff -= pos - temp;
                pos = temp;
                if (lbuff - pos > 0)
                    printf("\033[%ldD", lbuff - pos);
            }
        }
        else if (c == '\n') // Enter key
        {
            break;
        }
        else
        {
            // Make sure the character is printable
            if (!(c >= 32 && c <= 126))
                goto FLUSH;

            // Make sure the buffer is not full
            // Store the character in the input buffer
            if (lbuff < buff_size - 1)
            {
                for (int i = lbuff; i > pos; i--)
                {
                    buff[i] = buff[i - 1];
                }
                buff[pos] = c;
                lbuff++;
                putchar(c);
                for (int i = pos + 1; i < lbuff; i++)
                {
                    putchar(buff[i]);
                }
                pos++;
                if (lbuff - pos > 0)
                    printf("\033[%ldD", lbuff - pos);
            }
        }

    FLUSH:
        // Flush output buffer to ensure character is printed immediately
        fflush(stdout);
    }
    putchar('\n');

    restore_terminal_mode();
    return lbuff;
}
