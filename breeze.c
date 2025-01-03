// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

// DEFINES
#define BREEZE_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

// DATA

struct editorConfig { 
    int screenrows;
    int screencols;
    struct termios orig_termios;
} E;

// TERMINAL
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) 
        die("tcsetattr");
}

//  to turn off echoing...
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disableRawMode); //  disable raw mode at exit
    struct termios raw = E.orig_termios;

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

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

//  INPUT
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

//  OUTPUT
void editorDrawRows() {
    for (int y = 0; y < E.screenrows; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}


void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

//  INIT
void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == 1) 
        die("getWindowSize");
}
int main() {
    // printf("Hello! Breeze is a Text Editor written in C\nPress 'q' to quit!\n\n");
    enableRawMode();
    initEditor();
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