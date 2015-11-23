#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>
#include <fcntl.h>
#include <ncurses.h>
#include "cmd_process.h"

using namespace Robot;

#define VTANSI_ATTRIB_RESET 0
#define VTANSI_ATTRIB_BOLD 1
#define VTANSI_ATTRIB_DIM 2
#define VTANSI_ATTRIB_UNDERLINE 4
#define VTANSI_ATTRIB_BLINK 5
#define VTANSI_ATTRIB_REVERSE 7
#define VTANSI_ATTRIB_INVISIBLE 8

#define VTANSI_FG_BLACK 30
#define VTANSI_FG_RED 31
#define VTANSI_FG_GREEN 32
#define VTANSI_FG_YELLOW 33
#define VTANSI_FG_BLUE 34
#define VTANSI_FG_MAGENTA 35
#define VTANSI_FG_CYAN 36
#define VTANSI_FG_WHITE 37
#define VTANSI_FG_DEFAULT 39

#define VTANSI_BG_BLACK 40
#define VTANSI_BG_RED 41
#define VTANSI_BG_GREEN 42
#define VTANSI_BG_YELLOW 43
#define VTANSI_BG_BLUE 44
#define VTANSI_BG_MAGENTA 45
#define VTANSI_BG_CYAN 46
#define VTANSI_BG_WHITE 47
#define VTANSI_BG_DEFAULT 49

extern LinuxMotionTimer linuxMotionTimer;
int Col = STP7_COL;
int Row = ID_1_ROW;
int Old_Col;
int Old_Row;
bool bBeginCommandMode = false;
bool bEdited = false;
int indexPage = 1;
Action::PAGE Page;
Action::STEP Step;


int _getch()
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

int kbhit(bool bPushed)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF && bPushed == true)
  {
    ungetc(ch, stdin);
    return 1;
  }
  
	if(ch != EOF && bPushed == false)
		return 1;
  
	return 0;
}

struct termios oldterm, new_term;
void set_stdin(void)
{
	tcgetattr(0,&oldterm);
	new_term = oldterm;
	new_term.c_lflag &= ~(ICANON | ECHO | ISIG); // 의미는 struct termios를 찾으면 됨.
	new_term.c_cc[VMIN] = 1;
	new_term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &new_term);
}

void reset_stdin(void)
{
	tcsetattr(0, TCSANOW, &oldterm);
}
