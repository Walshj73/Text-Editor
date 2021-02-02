/* Program name : Text editor 
 * File name : main.c */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
}; /* End of editorConfig struct */

struct editorConfig E;

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
} /* End of die function */

void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
} /* End of disableRawMode function  */

void enableRawMode() {
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
		die("tcgetattr");

	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
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

int getCursorPosition(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;

	if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
		return -1;

	while(i < sizeof(buf) - 1) {
		if(read(STDIN_FILENO, &buf[i], 1) != 1)
			break;
		if(buf[i] == 'R')
			break;
		i++;
	} /* End of while loop */

	buf[i] = '\0';
	if(buf[0] != '\x1b' || buf[1] != '[')
		return -1;

	if(sscanf(&buf[2], "%d;%d", rows, cols) != 2)
		return -1;

	editorReadKey();
	return -1;
} /* End of getCursorPosition function */

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;

	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
			return -1;
		return getCursorPosition(rows, cols);
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	} /* End of if-else statement */
} /* End of getWindowSize function */

struct abuf {
	char *b;
	int len;
}; /* End of abuf struct */

#define ABUF_INIT {NULL, 0}

void editorDrawRows() {
	int y;
	for(y = 0; y < E.screenrows; y++) {
		write(STDOUT_FILENO, "~", 1);

		if(y < E.screenrows - 1) {
			write(STDOUT_FILENO, "\r\n", 2);
		} /* End of if statement */
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

void initEditor() {
	if(getWindowSize(&E.screenrows, &E.screencols) == -1)
		die("getWindowSize");
} /* End of initEditor function */

int main(int argc, char** argv) {
	enableRawMode();
	initEditor();

	while(1) {
		editorRefreshScreen();
		editorProcessKeypress();
	} /* End of while statement  */
    return 0;
} /* End of main function */
