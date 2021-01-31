/* Program name : Text editor 
 * File name : main.c */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct termios orig_termios;

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
} /* End of die function */

void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
} /* End of disableRawMode function  */

void enableRawMode() {
	if(tcgetattr(STDIN_FILENO, &orig_termios) == -1)
		die("tcgetattr");

	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN  | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
} /* End of enableRawMode function */

char editorReadKey() {
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if(nread == -1 && errno != EAGAIN)
			die("read");
	} /* End of while statement */
	return c;
} /* End of editorReadKey function */

void editorDrawRows() {
	int y;
	for(y = 0; y < 24; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	} /* End of for loop */
} /* End of editorDrawRows function */

void editorRefreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
} /* End of editorRefreshScreen function */

void editorProcessKeypress() {
	char c = editorReadKey();

	switch(c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	} /* End of switch statement */
} /* End of editorProcessKeyPress function */

int main(int argc, char** argv) {
	enableRawMode();

	while(1) {
		editorRefreshScreen();
		editorProcessKeypress();
	} /* End of while statement  */
    return 0;
} /* End of main function */