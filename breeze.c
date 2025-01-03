// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

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
    /*
        OPOST comes from <termios.h>.
        O means it’s an output flag, and I assume POST stands for “post-processing of output”
    */
    raw.c_lflag |= (CS8);
    raw.c_lflag &= ~(IXON | ECHO | ICANON | IEXTEN | ISIG);
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
   /*
        ICRNL comes from <termios.h>. 
        The I stands for “input flag”, CR stands for “carriage return”, and NL stands for “new line”.
   */
    /*
        When BRKINT is turned on, a break condition will cause a SIGINT signal to be sent to the program, like pressing Ctrl-C.
        INPCK enables parity checking, which doesn’t seem to apply to modern terminal emulators.
        ISTRIP causes the 8th bit of each input byte to be stripped, meaning it will set it to 0. This is probably already turned off.
        CS8 is not a flag, it is a bit mask with multiple bits, which we set using the bitwise-OR (|) operator unlike all the flags we are turning off. It sets the character size (CS) to 8 bits per byte. On my system, it’s already set that way.
    */

    //   timeout for read()
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
        die("tcsetattr");
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

//  INIT
int main() {
    printf("Hello! Breeze is a Text Editor written in C\nPress 'q' to quit!\n\n");

    enableRawMode();
    // char c;
    // while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    //     // disabling key press
    //     if (iscntrl(c)) {
    //         printf("%d\r\n", c);
    //     } else {
    //         printf("%d ('%c')\r\n", c, c);
    //     }
    // }

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }

        if (c == 'q')
            break;
    }


    return 0;
}