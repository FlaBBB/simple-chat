#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

// Function to set terminal attributes for non-canonical mode
void set_terminal_mode() {
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
void restore_terminal_mode() {
    // Get the current terminal attributes
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);

    // Restore canonical mode and echo
    t.c_lflag |= ICANON | ECHO;

    // Set the new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void getWindowsSize(int *row, int *col) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return;
    }
    *row = ws.ws_row;
    *col = ws.ws_col;
}