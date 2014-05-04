
#include "nc.h"
#include <curses.h>
#include <string.h>
#include <stdio.h>
FILE *mouselog;
/* ncurses stuff */

/*WINDOW *main;
WINDOW *callback;
WINDOW *shell;
WINDOW *result;*/
char buf[1025];

void nc_show_help()
{
    fprintf(stdout,"HELP\n===\n");
    fprintf(stdout,"q = quit\n");
    fprintf(stdout,"r = Read lib (no clearing)\n");
    fprintf(stdout,"c = Set configuration\n");
    fprintf(stdout,"s = Save configuration\n");
    fprintf(stdout,"l = Load configuration\n");
    fprintf(stdout,"g = Show configuration\n");
    fprintf(stdout,"x = Clear lib\n");
    fprintf(stdout,"\nMultiple commands can be put together\n(rrrq = read three times then quit.\n");
    //wrefresh(stdout);			
}

char nc_getc() {
    //wrefresh(lgwin);
    if (buf[0] == '\0') {
	fprintf(stdout,":");
	//wrefresh(stdout);
        fgets(buf, 1024, stdin);
	buf[strlen(buf)-1]='\0'; 
    }
    char ch = buf[0];
    for (unsigned int i=0;i<strlen(buf);i++)
	buf[i] = buf[i+1];
    return ch;
}

void nc_init()
{
    /*reasigne stderr */
 //   strcpy(buf, "h");
  //  initscr();
  //  keypad(stdscr, TRUE);
  //  nonl();
   // cbreak();
   // echo();
  //  start_color();
   /* cbwinb = newwin(LINES/2, COLS/2, 0,0);
    rswinb = newwin(LINES/2, COLS/2, 0, COLS/2);
    stdoutb = newwin(LINES/2, COLS/2, LINES/2,0);
    lgwinb = newwin(LINES/2, COLS/2, LINES/2,COLS/2);
    box(cbwinb, 0,0);
    mvfprintf(cbwinb,0,2,"Callback log");
    box(rswinb, 0,0);
    mvfprintf(rswinb,0,2,"Mose info");
    box(stdoutb, 0,0);
    mvfprintf(stdoutb,0,2,"LibShell");
    box(lgwinb, 0,0);
    mvfprintf(lgwinb,0,2,"ErrorLog");*/
    //wrefresh(cbwinb);
    //wrefresh(rswinb);
    //wrefresh(stdoutb);
    //wrefresh(lgwinb);
    mouselog = fopen("mouse.log", "w");
/*    cbwin = newwin(LINES/2-2, COLS/2-2, 1,1);
    rswin = newwin(LINES/2-2, COLS/2-2, 1, COLS/2+1);
    stdout = newwin(LINES/2-2, COLS/2-2, LINES/2+1,1);
    lgwin = newwin(LINES/2-2, COLS/2-2, LINES/2+1,COLS/2+1);
    scrollok(cbwin, TRUE);
    scrollok(rswin, TRUE);
    scrollok(stdout, TRUE);
    scrollok(lgwin, TRUE);*/
}

void nc_result(LMiceResult *result) {
    //wmove(rswin, 0,0);
    LMiceData *data;
    fprintf(mouselog, "Read of %d mouses in %lld usec\n", result->count, result->time);
    for(int i=0; i<result->count; i++) {
	data = result->data[i];
        fprintf(stdout, "res: %d:", i);
        fprintf(stdout, "(%3.3f, %3.3f) ", data->move_x, data->move_y);
        fprintf(stdout, "(%3d, %3d) ", data->raw_x, data->raw_y);
        fprintf(stdout, "overflows (%d, %d) ", data->overflow_x, data->overflow_y);
        fprintf(stdout, "read %d", data->reads);
	if (data->reads) {
            fprintf(stdout, "avgtime is %lld \n", result->time/data->reads);
	fprintf(mouselog, 
		"%d: move=(%3.3f, %3.3f) raw=(%3d, %3d) overflows=(%d, %d) reads=%d avgtime=%lld     \n", 
		i, data->move_x, data->move_y, 
		data->raw_x, data->raw_y, 
		data->overflow_x, data->overflow_y, 
		data->reads, result->time/data->reads);
	} else { 
            fprintf(stdout, "avgtime is 0       \n");
	fprintf(mouselog, 
		"%d: move=(%3.3f, %3.3f) raw=(%3d, %3d) overflows=(%d, %d) reads=%d avgtime=0\n", 
		i, data->move_x, data->move_y, 
		data->raw_x, data->raw_y, 
		data->overflow_x, data->overflow_y, 
		data->reads);
	} 
	    
    }
    //wrefresh(stdout);
}

void nc_error(char *errstr)
{
    //wmove(lgwin, 0,0);
    fprintf(stdout, "err:%s", errstr);
    //wrefresh(lgwin);
}
void nc_uninit()
{
    /*delwin(cbwin);
    delwin(stdout);
    delwin(stdout);
    delwin(lgwin);
    endwin();*/
}

