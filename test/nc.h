
#ifndef _NC_H
#define _NC_H

#include <curses.h>
#include <liblmice.h>

/* ncurses stuff */

WINDOW *cbwinb;
WINDOW *shwinb;
WINDOW *rswinb;
WINDOW *lgwinb;
WINDOW *cbwin;
WINDOW *shwin;
WINDOW *rswin;
WINDOW *lgwin;

void nc_init();
void nc_result(LMiceResult *data);
void nc_error(char *errstr);
void nc_uninit();
void nc_show_help();
char nc_getc();

#endif
