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
	    fprintf(stdout, "err: %s\n", er);
//	    wrefresh(lgwin);
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
    fprintf(stdout, "call: Callback signal : %d : %d : %lld\n", signal, idata, result->time);
    nc_result(result);
    lmice_result_copy(lastres, result);
    //wrefresh(cbwin);
    return false;
}

int cb2(LMiceSignal signal, int idata, LMiceResult *result, void *data)
{
    data = data;
    //fprintf(stdout, "call: Callback signal : %d : %d : %lld\n", signal, idata, result->time);
    nc_result2(result);
    lmice_result_copy(lastres, result);
    //wrefresh(cbwin);
    return false;
}

int main() {
    nc_init();
    file = freopen("lib.log", "w", stderr);
    signal(SIGINT, finish);
    int mice = lmice_init(LMICE_SYSTEM_DEVFS); /* Set plugintype */
    if (mice < 0) {
	finish(0);
    }
    fprintf(stdout, "Init lib found %d mice\n", mice);
    
    pthread_t thread;
    pthread_create(&thread, NULL, err_log, NULL);
    //wrefresh(shwin);
    lastres = lmice_result_new();
    lmice_connect(LMICE_CB_NEW_DATA, cb2, NULL);
    lmice_connect(LMICE_CB_OVERFLOW, cb2, NULL);
    lmice_timer_set(100);

    char ch = nc_getc();
    bool ok = true;
    LMiceResult *result = lmice_result_new();
    while (ch != 'q' && ch != 'Q') {
	switch (ch) {
	    case 'c':
	    case 'C':
		fprintf(stdout, "config\n");
		ok = lmice_config_set(lastres);
		break;
	    case 's':
	    case 'S':
		fprintf(stdout, "Writing configuration\n");
		ok = lmice_config_save(NULL);
		break;
	    case 'l':
	    case 'L':
		fprintf(stdout, "Loading configuration\n");
		ok = lmice_config_load(NULL);
		break;
	    case 'r':
	    case 'R':
		fprintf(stdout, "Read\n");
		ok = lmice_read(result, false);
		if (!ok) 
		    nc_error("Read error \n");
		nc_result2(result);
		break;
		break;
	    case 'x':
	    case 'X':
		fprintf(stdout, "Clearing\n");
		lmice_clear();
		break;
	    case 'h':
	    case 'H':
		nc_show_help();
		break;
	    default:
		fprintf(stdout, "unknown command %c\n", ch);
		break;
	}
	if (!ok)
	    fprintf(stdout, "an error happend....\n");
	fprintf(stdout,"\n");
	ch = nc_getc();
    }
    lmice_result_delete(result);
    thok = 0;
    void *data = NULL;;
    pthread_join(thread, data);
    lmice_uninit(true);
    nc_uninit();
    return 0;
}


