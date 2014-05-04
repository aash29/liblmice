
#include "plmouse.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define DBG_FILE "plmouse.c"


/**@defgroup complugin Common files and functions that all plugin use.
 * @ingroup intlmice
 *
 * This functions are the standard plugins used by all plugins. When
 * developing a new plugin plmouse.h should be included. This will leave
 * mouse_pl_init() and mouse_pl_uninit() left for the new plugin to handle.
 *
 * @{
 */

/**
 * @brief distance calculation
 * */
#define DIST(x,y) sqrt((x)*(x) + (y)*(y))

/**
 * @brief The number of mouses the pluginsystem finds 
 *
 * This defaults to 0
 */
int mouse_count = 0;

/**
 * @brief The old timer time of the system
 */
struct timeval old_time;

/**
 * @brief Preinitializes the common infrastructure of the plugin.
 * 
 * Clears the plug-in with standard startup data
 */
void mouse_pl_preinit()
{
    DBG("pl_preinit()");
    mouse_count = 0;
    main_scale = 1;
    pthread_mutex_init(&main_mutex,NULL);
    mouse_has_data = false;
    gettimeofday(&old_time, NULL);
}

/**
 * @brief Clear mousedata collected by the plugin 
 */
void mouse_pl_clear()
{
    DBG("pl_clear()");
    int i=0;
    LOCK();
    for(;i<mouse_count;i++)
	lmice_data_clear(mouse[i]->data);
    UNLOCK();
}

/**
 * @brief read all mices and apply configuration.
 *
 * This function reads all mices and apply it's configuration.
 * Observ that result should never be null, that case is not handled, 
 * returns false, in this function since it already has been handled in 
 * lmice_read()
 *
 * @param result where to store the readings
 * @param clear if set to true the function will clear the internal mousedata for every mouse
 *	    after the read is compleat (this should be seen as defat)
 *
 * @return true on success, false on error
 */
int mouse_pl_read_all(LMiceResult *result, bool clear)
{
    DBG("pl_read_all()");
    if (!result) {
	ERR("No resultstorage");
	return false;
    }
    struct timeval new_time;
    gettimeofday(&new_time, NULL);
    result->time = ((long long)new_time.tv_usec - old_time.tv_usec) + 
	(long long)1000000 * ((long long)new_time.tv_sec - old_time.tv_sec);
    if (clear)
        memcpy(&old_time, &new_time, sizeof(struct timeval)); /* fast time copy */
    DBG("timer is %lld", result->time);
    /*make the result the same number of reads as the mouses */
    int i;
    if (result->count < mouse_count) {
	for (i=result->count; i< mouse_count; i++) 
	    result->data[i] = lmice_data_new();
    } else if (result->count > mouse_count) {
	for (i=result->count; i> mouse_count; i--) 
	    lmice_data_delete(result->data[i]);
    }
    result->count = mouse_count;
    LOCK();
    /* Scan true the mices, store and apply configuration on them all */
    for (i=0;i<mouse_count;i++) {
	DBG("i=%d mc=%d", i, mouse_count);
	lmice_data_copy(result->data[i], mouse[i]->data);
	result->data[i]->move_x = (
	        (float)result->data[i]->raw_x * cosf(mouse[i]->forward) + 
		(float)result->data[i]->raw_y * sinf(mouse[i]->forward)
		) * mouse[i]->scale;
	result->data[i]->move_y = (
		(float)result->data[i]->raw_x * cosf(mouse[i]->forward + PI/2) + 
		(float)result->data[i]->raw_y * sinf(mouse[i]->forward + PI/2)
		) * mouse[i]->scale;
	result->avg->raw_x += result->data[i]->raw_x;
	result->avg->raw_y += result->data[i]->raw_y;
	result->avg->move_x += result->data[i]->move_x;
	result->avg->move_y += result->data[i]->move_y;
	result->avg->reads += result->data[i]->reads;
    }
    result->avg->raw_x /= mouse_count;
    result->avg->raw_y /= mouse_count;
    result->avg->move_x /= mouse_count;
    result->avg->move_y /= mouse_count;
    result->avg->reads /= mouse_count;
    if (clear) {
	for (i=0;i<mouse_count;i++)
	    lmice_data_clear(mouse[i]->data);
	mouse_has_data = false;
    }
    UNLOCK();
    return true;
}

/**
 * @brief Read a selected mouse
 *
 * @param mousenr the mouse to read
 * @param data	where to store the information
 *
 * @return true on success, false on error.
 */
int mouse_pl_read(int mousenr, LMiceData *data)
{
    DBG("read()");
    LOCK();
    lmice_data_copy(data, mouse[mousenr]->data);
    UNLOCK();
    return true;
}

/**
 * @brief Check if any mouse has any new data
 *
 * @return true if there is new data in a mouse, false if there is not.
 */
bool mouse_pl_new_data()
{
    //DBG("new_data() returns %d", mouse_has_data);
    return mouse_has_data;
}

/** 
 * @brief Internal function to trim a string
 *
 * @param buf is the buffer to be trimmed
 *
 * @return true on sucess, false on error.
 */
bool mouse_buf_trim(char *buf)
{
    int i;
    /* remove cruft in the beginning */
    DBG("buf_trim_str(%s)", buf);
    while (buf[0] == ' ') {
	int len = strlen(buf);
	for (i=0;i<len-1;i++)
	    buf[i] = buf[i+1];
    }
    /* remove cruft at end */
    while (buf[strlen(buf)-1] == ' ' || buf[strlen(buf)-1] == '\n')
	buf[strlen(buf)-1] = '\0';
    return true;
}

/**
 * @brief The line the first line in a configurationfile must include 
 */
#define LMICE_FILE_V "libLMice fileversion 3"

/**
 * @brief load configuration from file
 *
 * @param filename the file to load from
 *
 * @return true on sucess, false on error
 */
bool mouse_pl_config_load(char *filename)
{
    DBG("pl_config_load()");
    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL) {
	ERR("Could not open file : %s", filename);
	return false;
    }
    char buf[1024] = "nothing";
    char *ch;
    if (fgets(buf, 1024, file) == NULL) {
	ERR("fgets could not read correctly");
	return false;
    }
    mouse_buf_trim(buf);
    if (strncmp(buf, LMICE_FILE_V, strlen(LMICE_FILE_V)) != 0) {
	WRN("Not a configuration file, \"%s\" == \"%s\" failed!", LMICE_FILE_V, buf);
	fclose(file);
	return false;
    }
    
    struct LMouse *cm = NULL;
    
    while (!feof(file)) {
	fgets(buf, 1024, file);
	ch = strchr(buf, '=');
	if (!ch)
	    continue;
	ch[0] = '\0';
	ch++;
	mouse_buf_trim(buf);
	mouse_buf_trim(ch);
#define IFSTR(x) if (strcmp(buf, (x)) == 0)
	IFSTR("mainscale")
	    main_scale = atof(ch);
	IFSTR("mousenr")
	    cm = mouse[atoi(ch)];
	IFSTR("mousename") {
	    int i;
	    for(i=0;i<mouse_count;i++) {
		if (strcmp(mouse[i]->filename, ch) == 0) 
		    cm = mouse[i];
	    }
	}
	if (cm) {
	    IFSTR("mousescale")
		cm->scale = atof(ch);
	    IFSTR("mouseforward")
		cm->forward = atof(ch);
	}
	    
#undef IFCFG
    }
    DBG("%s", buf);
    return true;
}

/**
 * @brief Save configuration using filename
 *
 * @param filename the filename to save
 *
 * @return true on sucess, false on error
 */
bool mouse_pl_config_save(char *filename)
{
    DBG("pl_config_save()");
    FILE *file;
    if (!(file = fopen(filename, "w"))) {
	ERR("Could not open file : %s", filename);
	return false;
    }
    fprintf(file, "%s\n", LMICE_FILE_V);
    fprintf(file, "mainscale=%f\n", main_scale);
    int i;
    for(i=0;i<mouse_count;i++) {
	fprintf(file, "mousenr=%d\n", i);
	if (mouse[i]->filename)
	    fprintf(file, "mousename=%s\n", mouse[i]->filename);
	fprintf(file, "mousescale=%f\n", mouse[i]->scale);
	fprintf(file, "mouseforward=%f\n", mouse[i]->forward);
    }
    fclose(file);
    return true;
}

/** 
 * @brief Use result to configure the mice
 *
 * @param result the data to be used to configure
 *
 * @return true on sucess, false on error.
 */
bool mouse_pl_config_set(LMiceResult *result)
{
    DBG("pl_config_set()");
    if (result->count != mouse_count) {
	ERR("Resultcount and mousecount does not mach");
	return false;
    }
    float max_dist=0;
    int i = 0;
    for(;i<result->count;i++) {
	
	if (max_dist < DIST(result->data[i]->raw_x, result->data[i]->raw_y))
	    max_dist = DIST(result->data[i]->raw_x, result->data[i]->raw_y);
    }
    /* set scale */
    for(i=0;i<result->count;i++) {
	//mouse[i]->forward = atan2f(result->data[i]->raw_x, -result->data[i]->raw_y);
	mouse[i]->forward = atan2f(result->data[i]->raw_y, result->data[i]->raw_x);
	if (result->data[i]->raw_x + result->data[i]->raw_y == 0) {
	    WRN("Did not set scale on mouse %i since it has not moved", i);
	    continue;
	}
	mouse[i]->scale = ((float)max_dist)/DIST(result->data[i]->raw_x, result->data[i]->raw_y);
	DBG("Scale for mouse %d is %.3f", i, mouse[i]->scale);
    }
    return true;
}

/* @} */

