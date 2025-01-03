// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

// DEFINES
#define BREEZE_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

// DATA
struct termios orig_termios;

// TERMINAL
void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) 
        die("tcsetattr");
}

//  to turn off echoing...
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disableRawMode); //  disable raw mode at exit
    struct termios raw = orig_termios;

    tcgetattr(STDERR_FILENO, &raw);
    raw.c_lflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON ); // disables Ctrl+S and Ctrl+Q
    raw.c_lflag &= ~(OPOST);
    raw.c_lflag |= (CS8);
    raw.c_lflag &= ~(IXON | ECHO | ICANON | IEXTEN | ISIG);
    
    //   timeout for read()
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
        die("tcsetattr");

}

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    return c;
}

//  INPUT
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

//  OUTPUT
void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

//  INIT
int main() {
    printf("Hello! Breeze is a Text Editor written in C\nPress 'q' to quit!\n\n");

    enableRawMode();
    while (1) {
        // char c = '\0';
        // if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
        //     die("read");
        // if (iscntrl(c)) {
        //     printf("%d\r\n", c);
        // } else {
        //     printf("%d ('%c')\r\n", c, c);
        // }

        // if (c == CTRL_KEY('q'))
        //     break;
        editorRefreshScreen();
        editorProcessKeypress();
    }


    return 0;
}