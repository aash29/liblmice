
#include "plmouse.h"
#include "devfs.h"
#include "../result.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/poll.h>

#define DBG_FILE "plugin/devfs.c"

/**@defgroup devfsplugin The devfs plugin specifics
 * @ingroup complugin
 *
 * Functions and variables that are specific for the devfs plugin
 * The devfs plug-in relies on one single thread that polls all mices
 * and stores mousemovement as soon as they are avalibel to the plug-in.
 *
 * @{
 */

struct LMouse *mouse_inits(char *filename);
void *mouse_poll(void *data);

/**
 * @brief the polling thread of the system
 */
pthread_t thread;

/**
 * @brief Command variable to stop the thread
 */
bool thread_run;

/**
 * @brief Initialization of the devfs plugin.
 *
 * This function tries to open all mouses found in
 * LMICE_DEVFS_PATH. 
 *
 * @return the number of mice found, negative on error.
 */
int mouse_pl_init()
{
    DBG("pl_init()");
    thread = 0;
    DIR *dir;
    dir = opendir(LMICE_DEVFS_PATH);
    if (!dir) {
	ERR("Could not open devfs path %s", LMICE_DEVFS_PATH);
	return -1;
    }
    thread = 0;
    LOCK();
    struct dirent *dirent;
    int cnt = 0;
    while ((dirent = readdir(dir))) {
	if (strncmp(dirent->d_name, LMICE_DEVFS_FILTER, 
		    strlen(LMICE_DEVFS_FILTER)-1) != 0) {
	    DBG("Filter out %s", dirent->d_name);
	    continue;
	}
	DBG("Loading %s", dirent->d_name);
	mouse[cnt] = mouse_inits(dirent->d_name);
	if (mouse[cnt])
	    cnt++;
    }
    closedir(dir);
    mouse_count = cnt;
    /* All mice are loaded, start thread */
    thread_run = true;
    pthread_create(&thread, NULL, mouse_poll, mouse);	/* All mice are loaded, start thread */
    UNLOCK();
    return cnt;
}


/**
 * @brief Initialization of a mouse
 *
 * This function setts up a mouse, and opens it for the polling thread
 *
 * @return a newly create LMouse structure, NULL on error
 */
struct LMouse *mouse_inits(char *filename)
{
    DBG("init(filename=%s)", filename);
    struct LMouse *mouse = malloc(sizeof(struct LMouse));
    mouse->filename = malloc(strlen(LMICE_DEVFS_PATH)+strlen(filename)+1);
    /* Set up path + filename */
    strcpy(mouse->filename, LMICE_DEVFS_PATH);
    if (mouse->filename[strlen(mouse->filename)-1] != '/')
	strcat(mouse->filename, "/");
    strcat(mouse->filename, filename);
    mouse->forward = 0;
    mouse->scale = 0;
    
    
    struct DevfsItem *system = malloc(sizeof(struct DevfsItem));
    mouse->system = system;
    system->fd = open(mouse->filename, O_RDONLY);
    if (system->fd <= 0) {
	ERR("Could not open file %s", mouse->filename);
	free(mouse);
	free(system);
	return NULL;
    }
    mouse->data = lmice_data_new();
    lmice_data_clear(mouse->data);

    return mouse;
}

/**
 * @brief Function called by the polling thread to read a mouse
 *
 * @param mouse the mouse to be read.
 */
void mouse_reader(struct LMouse *mouse) 
{
    /* Locking is done by callingfunction */
    signed char b[4];
    struct DevfsItem *system = (struct DevfsItem *) mouse->system;
    DBG("Read %d", system->fd);
    int rsize = read(system->fd, b, 4);
    
    if (rsize != 3) {
	WRN("Wrong read size, skiping result.");
	return;
    } else {
	DBG("readsize = %d", rsize);
    }
    if (!BTEST(b[0], ALWAYS_ONE)) {
	ERR("A zero was supposed to be one.");
	return;
    }
    DBG("Read oki %d %d", b[1], b[2]);
    signed char ovf = b[1];
    if (ovf & 0x80)
	ovf = -ovf;
    if ((b[0] & X_OVERFLOW) || ovf == 0x7F) {
	ERR("Overflow on X %02x %02x (%d, %d)", b[0] & X_OVERFLOW, ovf, b[1], b[2]);
	mouse->data->overflow_x++;
    }
    ovf = b[2];
    if (ovf % 0x80)
	ovf = -ovf;
    if ((b[0] & Y_OVERFLOW) || ovf == 0x7F) {
	ERR("Overflow on Y %02x %02x (%d, %d)", b[0] & Y_OVERFLOW, ovf, b[1], b[2]);
	mouse->data->overflow_y++;
    }
    mouse->data->raw_x += ((int) b[1]);
    mouse->data->raw_y += ((int) b[2]);
    mouse->data->reads++;
    mouse_has_data = true;
}

/**
 * @brief the polling thread function
 *
 * Threadfunction that polls all opened mouses and stores there data
 * The thread runs until thread_run is set to false.
 *
 * @param data	pthread data,  not used.
 *
 * @return this function always returns NULL to comply with the phtread system
 */
void *mouse_poll(void *data)
{
    DBG("poll()");
    REMOVE_CC_WARNING_ON(data);
/*    ms = (struct LMouse *[LMICE_MAX_MICE]) data;*/
    struct pollfd polls[mouse_count];
    int i=0;
    for (;i< mouse_count;i++) {
	struct DevfsItem *system = (struct DevfsItem *) mouse[i]->system;
	
	polls[i].fd = system->fd;
	polls[i].events = POLLIN | POLLPRI;
    }
    int mnr;
    while (thread_run) {
	mnr = poll(polls, mouse_count, 100);
	if (mnr < 0) {
	    WRN("error in poll");
	    continue;
	}
	if (mnr == 0) {
	    /* timeout */
	    continue;
	}
	LOCK();
	for (i=0;i<mouse_count;i++) {
	    if (polls[i].revents > 0) {
		DBG("Read mouse %d", i);
		mouse_reader(mouse[i]);
		/* signal callback? */
	    }
	}
	UNLOCK();
    }
    return NULL;
}

/**
 * @brief Close all mouses and end the polling thread
 */
void mouse_pl_close()
{
    DBG("close()");
    DBG("Close thread");
    thread_run = false;
    void *data = NULL;
    pthread_join(thread, data);
    int i=0;
    LOCK();
    for (;i<mouse_count;i++) {
	struct DevfsItem *system = (struct DevfsItem *) mouse[i]->system;
	close(system->fd);
	free(system);
	lmice_data_delete(mouse[i]->data);
	free(mouse[i]->filename);
	free(mouse[i]);	
    }
    UNLOCK();
    pthread_mutex_destroy(&main_mutex);
}

/* @} */
