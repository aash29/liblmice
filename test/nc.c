
#include "nc.h"
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
FILE *mouselog;
FILE *indat;
/* ncurses stuff */

/*WINDOW *main;
WINDOW *callback;
WINDOW *shell;
WINDOW *result;*/
char buf[1025];

long long rs[100];
long long of[100];

void nc_show_help()
{
    wprintw(shwin,"HELP\n===\n");
    wprintw(shwin,"q = quit\n");
    wprintw(shwin,"r = Read lib (no clearing)\n");
    wprintw(shwin,"c = Set configuration\n");
    wprintw(shwin,"s = Save configuration\n");
    wprintw(shwin,"l = Load configuration\n");
    wprintw(shwin,"g = Show configuration\n");
    wprintw(shwin,"x = Clear lib\n");
    wprintw(shwin,"\nMultiple commands can be put together\n(rrrq = read three times then quit.\n");
    wrefresh(shwin);			
}

char nc_getc() {
    wrefresh(lgwin);
    if (buf[0] == '\0') {
	wprintw(shwin,":");
	wrefresh(shwin);
        wgetnstr(shwin,buf,1024);
    }
    char ch = buf[0];
    for (unsigned int i=0;i<strlen(buf);i++)
	buf[i] = buf[i+1];
    return ch;
}

void nc_init()
{
    /*reasigne stderr */
    int i;
    for(i=0;i<100;i++){
	rs[i] = 0;
	of[i] = 0;
    }
    strcpy(buf, "h");
    initscr();
    keypad(stdscr, TRUE);
    nonl();
    cbreak();
    echo();
    start_color();
    cbwinb = newwin(LINES/2, COLS/2, 0,0);
    rswinb = newwin(LINES/2, COLS/2, 0, COLS/2);
    shwinb = newwin(LINES/2, COLS/2, LINES/2,0);
    lgwinb = newwin(LINES/2, COLS/2, LINES/2,COLS/2);
    box(cbwinb, 0,0);
    mvwprintw(cbwinb,0,2,"Callback log");
    box(rswinb, 0,0);
    mvwprintw(rswinb,0,2,"Mose info");
    box(shwinb, 0,0);
    mvwprintw(shwinb,0,2,"LibShell");
    box(lgwinb, 0,0);
    mvwprintw(lgwinb,0,2,"ErrorLog");
    wrefresh(cbwinb);
    wrefresh(rswinb);
    wrefresh(shwinb);
    wrefresh(lgwinb);
    mouselog = fopen("mouse.log", "w");
    indat = fopen("indat.log", "w");
    cbwin = newwin(LINES/2-2, COLS/2-2, 1,1);
    rswin = newwin(LINES/2-2, COLS/2-2, 1, COLS/2+1);
    shwin = newwin(LINES/2-2, COLS/2-2, LINES/2+1,1);
    lgwin = newwin(LINES/2-2, COLS/2-2, LINES/2+1,COLS/2+1);
    scrollok(cbwin, TRUE);
    scrollok(rswin, TRUE);
    scrollok(shwin, TRUE);
    scrollok(lgwin, TRUE);
}

void nc_result(LMiceResult *result) {
    wmove(rswin, 0,0);
    LMiceData *data;
    fprintf(mouselog, "Read of %d mouses in %lld usec\n", result->count, result->time);
    for(int i=0; i<result->count; i++) {
	data = result->data[i];
        wprintw(rswin, "%d:", i);
        wprintw(rswin, "(%3.3f, %3.3f) ", data->move_x, data->move_y);
        wprintw(rswin, "(%3d, %3d) \n", data->raw_x, data->raw_y);
        wprintw(rswin, "overflows (%d, %d) ", data->overflow_x, data->overflow_y);
        wprintw(rswin, "read %d\n", data->reads);
	if (data->reads) {
            wprintw(rswin, "avgtime is %ld\n", result->time/data->reads);
	fprintf(mouselog, 
		"%d: move=(%3.3f, %3.3f) raw=(%3d, %3d) overflows=(%d, %d) reads=%d avgtime=%lld     \n", 
		i, data->move_x, data->move_y, 
		data->raw_x, data->raw_y, 
		data->overflow_x, data->overflow_y, 
		data->reads, result->time/data->reads);
		int nr;
		if (abs(data->raw_x) > abs(data->raw_y)) {
		    nr = abs(data->raw_x)/500;
		    of[nr] += data->overflow_x;
		} else {
		    nr = abs(data->raw_y)/500;
		    of[nr] += data->overflow_y;
		}
		rs[nr] += data->reads;
	} else { 
            wprintw(rswin, "avgtime is 0       \n");
	    fprintf(mouselog, 
		"%d: move=(%3.3f, %3.3f) raw=(%3d, %3d) overflows=(%d, %d) reads=%d avgtime=0\n", 
		i, data->move_x, data->move_y, 
		data->raw_x, data->raw_y, 
		data->overflow_x, data->overflow_y, 
		data->reads);
	} 
	    
    }
    wrefresh(rswin);
}

void nc_error(char *errstr)
{
    wmove(lgwin, 0,0);
    wprintw(lgwin, "%s", errstr);
    wrefresh(lgwin);
}
void nc_uninit()
{
    FILE *f = fopen("data.log","w");
    long long i;
    for (i=0;i<100;i++) {
	if (rs[i] > 0)
	    fprintf(f,"%lld, %lld, %lld, %lld\n", i*500, of[i], rs[i], (of[i]*100)/rs[i]);
    }
    fclose(f);
    delwin(cbwin);
    delwin(rswin);
    delwin(shwin);
    delwin(lgwin);
    endwin();
}

