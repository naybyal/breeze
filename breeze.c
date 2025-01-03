#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

//  to turn off echoing...
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode); //  disable raw mode at exit
    struct termios raw = orig_termios;

    tcgetattr(STDERR_FILENO, &raw);
    // raw.c_lflag &= ~(IXON); // disables Ctrl+S and Ctrl+Q
    raw.c_lflag &= ~(IXON |ECHO | ICANON | ISIG);
    /*  
        ICANON comes from <termios.h>. 
        Input flags (the ones in the c_iflag field) generally start with I like ICANON does. 
        However, ICANON is not an input flag, it’s a “local” flag in the c_lflag field. 
    */
    /*
        ISIG comes from <termios.h>. 
        Like ICANON, it starts with I but isn’t an input flag.
        Now Ctrl-C can be read as a 3 byte and Ctrl-Z can be read as a 26 byte.
        This also disables Ctrl-Y on macOS, which is like Ctrl-Z except it waits for the program to read input before suspending it.
    */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    /*
        Terminal attributes can be read into a termios struct by tcgetattr(). 
        After modifying them, you can then apply them to the terminal using tcsetattr(). 
        The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.

        The c_lflag field is for “local flags”. A comment in macOS’s <termios.h> describes it as a “dumping ground for other state”. 
        So perhaps it should be thought of as “miscellaneous flags”. 
        The other flag fields are c_iflag (input flags), c_oflag (output flags), and c_cflag (control flags), all of which we will have to modify to enable raw mode.

        ECHO is a bitflag, defined as 00000000000000000000000000001000 in binary. 
        We use the bitwise-NOT operator (~) on this value to get 11111111111111111111111111110111. 
        We then bitwise-AND this value with the flags field, which forces the fourth bit in the flags field to become 0, and causes every other bit to retain its current value. 
        Flipping bits like this is common in C.
        
    */

}

int main() {
    printf("Hello! Breeze is a Text Editor written in C\nPress 'q' to quit!\n\n");

    enableRawMode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        // disabling key press
        if (iscntrl(c)) {
            printf("%d\n", c);
        } else {
            printf("%d ('%c')\n", c, c);
        }
    }
    return 0;
}