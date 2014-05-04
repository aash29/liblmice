#define _XOPEN_SOURCE 500 /*for usleep */

#include <liblmice.h>
#include "nc.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

FILE *file;

LMiceResult *lastres;

int thok;
void *err_log(void *data)
{
    data = data;
    thok = 1;
    while (thok) {
	char *er = lmice_error_get();
	if (er) {
	    wprintw(lgwin, "%s\n", er);
	    wrefresh(lgwin);
	    free(er);
	} else {
	    usleep(100);
	}
    }
    return data;
}

static void finish(int sig)
{   
    sig=sig;
    nc_uninit();
    exit(0);
}

int cb(LMiceSignal signal, int idata, LMiceResult *result, void *data)
{
    data = data;
    wprintw(cbwin, "Callback signal : %d : %d : %lld\n", signal, idata, result->time);
    nc_result(result);
    lmice_result_copy(lastres, result);
    wrefresh(cbwin);
    return false;
}

int main() {
    nc_init();
    file = freopen("lib.log", "w", stderr);
    signal(SIGINT, finish);
    int mice = lmice_init(LMICE_SYSTEM_LIBUSB); /* Set plugintype */
    if (mice < 0) {
	finish(0);
    }
    wprintw(shwin, "Init lib found %d mice\n", mice);
    
    pthread_t thread;
    pthread_create(&thread, NULL, err_log, NULL);
    wrefresh(shwin);
    lastres = lmice_result_new();
    lmice_connect(LMICE_CB_NEW_DATA, cb, NULL);
    lmice_connect(LMICE_CB_OVERFLOW, cb, NULL);
    lmice_timer_set(5000000);

    char ch = nc_getc();
    bool ok = true;
    while (ch != 'q' && ch != 'Q') {
	switch (ch) {
	    case 'c':
	    case 'C':
		wprintw(shwin, "config");
		ok = lmice_config_set(lastres);
		break;
	    case 's':
	    case 'S':
		wprintw(shwin, "Writing configuration");
		ok = lmice_config_save(NULL);
		break;
	    case 'l':
	    case 'L':
		wprintw(shwin, "Loading configuration");
		ok = lmice_config_load(NULL);
		break;
	    case 'r':
	    case 'R':
		wprintw(shwin, "Read");
		ok = lmice_read(lastres, false);
		if (!ok) 
		    nc_error("Read error");
		nc_result(lastres);
		break;
	    case 'x':
	    case 'X':
		wprintw(shwin, "Clearing");
		lmice_clear();
		break;
	    case 'h':
	    case 'H':
		nc_show_help();
		break;
	    default:
		wprintw(shwin, "unknown command %c", ch);
		break;
	}
	if (!ok)
	    wprintw(shwin, "an error happend....\n");
	wprintw(shwin,"\n");
	ch = nc_getc();
    }
    lmice_result_delete(lastres);
    thok = 0;
    void *data = NULL;;
    pthread_join(thread, data);
    lmice_uninit(true);
    nc_uninit();
    return 0;
}


