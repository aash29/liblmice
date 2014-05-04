#define _XOPEN_SOURCE 500 /*for usleep */

#include "lmice.h"
#include "result.h"
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <sys/poll.h>
#include <unistd.h>
#include <dlfcn.h>

#define DBG_FILE "lmice.c"

/** @defgroup intlmice Internals of lmice.c and lmice.h
 * 
 * These functions are the internal functions and types that 
 * are not exported outside of the general part of the library.
 * 
 * @{
 */

/**
 * @brief common initialization of the plugin 
 * 
 * Functionpointer to the preinit function in the plugin
 */
void (*mouse_plugin_preinit)();

/**
 * @brief initialization of the plugins specific behavior parts 
 *
 * Functionpointer to the init function in the plugin.
 * 
 * @return the number of mice found and initialized.
 */
int (*mouse_plugin_init)();

/**
 * @brief Read all the mice-data from the plugin
 *
 * Functionpointer to the read_all function in the plugin.
 * 
 * @param result    Where to store the data from the mice 
 * @param clear	    If the mousedata should be cleard after the read.
 *
 * @return true on sucess and false on error
 */
int (*mouse_plugin_read_all)(LMiceResult *result, bool clear);

/**
 * @brief ask the plugin if there is any new data to retrive.
 *
 * Functionpointer to the new_data function in the plugin.
 *
 * @return true if there are data to retrive from the mice, false if the counters are zero.
 */
bool (*mouse_plugin_new_data)();

/**
 * @brief Load configuration from a configurationfile
 *
 * Functionpointer to the config_load function in the plugin.
 * 
 * @param filename the file to be read.
 *
 * @return true on sucess and false on error
 */
bool (*mouse_plugin_config_load)(char *filename);

/**
 * @brief Save current pluginconfiguration to filename
 *
 * Functionpointer to the config_save function in the plugin.
 *
 * @param filename where to save the file
 * 
 * @return true on sucess, false on error.
 */
bool (*mouse_plugin_config_save)(char *filename);

/**
 * @brief Use result to set configuration of the mice.
 * 
 * Functionpointer to the config_set function in the plugin.
 *
 * @param result the data to be used.
 *
 * @return true on sucess, false on error.
 */
bool (*mouse_plugin_config_set)(LMiceResult *result);

/**
 * @brief Clear the mice.
 * 
 * Functionpointer to the clear function in the plugin.
 *
 * This function clear the internal counters of the plugin
 */
void (*mouse_plugin_clear)();

/**
 * @brief Close the plugin
 * 
 * Functionpointer to the close function in the plugin.
 *
 * Uninitialize the plugin
 */
void (*mouse_plugin_close)();


/** 
 * @brief Plugin Loader.
 * 
 * This function gets called by the lmice_init function and loads the selected plugin and it's 
 * symbols.
 *
 * @param filename  The filename of the plubin to load. The file must be placed in PLUGINDIR.
 *
 * @return true on sucess and false on error
 */
bool lmice_load(char *filename)
{
    DBG("load(filename=%s)", filename);
    char rfilename[1024];
    strcpy(rfilename, PLUGINDIR);
    strcat(rfilename, "/");
    strcat(rfilename, filename);
    DBG("Loading..");
    /* load plugin file */
    lmice->plhandle = dlopen(rfilename, RTLD_LAZY);
    /* Grab all handles */
    if (!lmice->plhandle) {
	WRN("Could not open file %s", rfilename);
	fprintf(stderr, dlerror());
	return false;
    }
    if (!(mouse_plugin_preinit = dlsym(lmice->plhandle, "mouse_pl_preinit"))) {
	WRN("Could not set plugin preinit symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_init = dlsym(lmice->plhandle, "mouse_pl_init"))) {
	WRN("Could not set plugin init symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_read_all = dlsym(lmice->plhandle, "mouse_pl_read_all"))) {
	WRN("Could not set plugin read_all symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_new_data = dlsym(lmice->plhandle, "mouse_pl_new_data"))) {
	WRN("Could not set plugin new_data symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_config_load = dlsym(lmice->plhandle, "mouse_pl_config_load"))) {
	WRN("Could not set plugin config_load symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_config_save = dlsym(lmice->plhandle, "mouse_pl_config_save"))) {
	WRN("Could not set plugin config_save symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_config_set = dlsym(lmice->plhandle, "mouse_pl_config_set"))) {
	WRN("Could not set plugin config_set symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_clear = dlsym(lmice->plhandle, "mouse_pl_clear"))) {
	WRN("Could not set plugin clear symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    if (!(mouse_plugin_close = dlsym(lmice->plhandle, "mouse_pl_close"))) {
	WRN("Could not set plugin close symbol: %s", dlerror());
	dlclose(lmice->plhandle);
	return false;
    }
    mouse_plugin_preinit(); /* Call the preinit structure */
    return true;
}

/** 
 * @brief The basic internal of the library 
 */
struct LMice *lmice = NULL;


/** @brief The readthread function
 *
 * This function continusly reads the current states of the mices and stores 
 * stores them in the general lib. This clears the mices. Thread is started
 * stopped with the lmice_timer_set() function.
 *
 * @param data	Not used, needed by pthread_create
 *
 * @return  Not used, data for pthread_join
 */
void *lmice_thread_read(void *data)
{
    DBG("thread_timer()");
    REMOVE_CC_WARNING_ON(data);
    useconds_t uwait_int = lmice->uwait; /* removes a rase */
    while (uwait_int > 0) {
	/* wait */
	usleep(uwait_int);
	if (mouse_plugin_new_data())
	    lmice_callback(LMICE_CB_NEW_DATA, uwait_int);
	uwait_int = lmice->uwait;
    }
    return NULL;
}

/** Initiate a signal callback.
 *
 * This function vill call a callback function if it exitst for the specified
 * signal. 
 *
 * @param signal    The signal that should be used
 * @param ival	    The ival that should be sent
 * 
 * @return  Returns the callback functions returnvalue or false if there
 *	    is no callback currently for that signal.
 */
int lmice_callback(LMiceSignal signal, int ival) {
    DBG("callback(signal=%d, ival=%d)", signal, ival);
    if (lmice->callback[signal].function != NULL) {
	LMiceResult *tmpr = lmice_result_new();
	mouse_plugin_read_all(tmpr, true);
	DBG("call to callbackfunction %d", signal);
	int retval = lmice->callback[signal].function(signal, ival, tmpr,
		lmice->callback[signal].data);
	lmice_result_delete(tmpr);
	return retval;
    }
    return false;
}
	    

/* @} */

/** @defgroup liblmice Library Interface
 * 
 * Exported functions that can be used by including liblmice.h. 
 * lmice_result_* and lmice_error function works even if the lmice 
 * library has not been initialized. 
 * 
 * All other function demands that lmice_init() is first called.
 * 
 * To change plugin during running of the program just call 
 * lmice_uninit();
 * lmice_init(new_plugin);
 * @{ 
 */


/** 
 * @brief Initialize the liblmice system
 *
 * This must be the fist function of the liblmice functions that gets called. 
 * This function can be called multple times, but it will only reply with 
 * the number of previosly found mouses after firs execution as long as
 * lmice_uninit() has not been called.
 * 
 * To test if the library has not been initialized and if it has not, initialize
 * it just run
 *
 * if (lmice_init(SYSTEM) < 0)
 *	exit(1);
 *
 * @param system    The subsystem (plug-in) to load. If not proporly specified a 
 *		    dummy system will be loaded. Se
 *
 * @return The number of mice found, negitve value on error.
 */
int lmice_init(LMiceSystemType system)
{ 
    /* Only public function without INIT_TEST() since that would look somewhat stupid.*/
    DBG("init(system=%d)",system);
    if (lmice)
	return lmice->count;
    lmice = malloc(sizeof(struct LMice));
    lmice->system = system;
    lmice->scale = 1;
    lmice->count = 0;
    lmice->uwait = 0;
    lmice->thread_read = 0;
    
    strcpy(lmice->error, "");
    for (int i=0;i<NUM_LMICE_CB;i++) {
	lmice->callback[i].function = NULL;
    }
    bool sysloaded = false;
    switch (system) {
	case LMICE_SYSTEM_DEVFS:
	    sysloaded = lmice_load("devfs.so");
	    break;
	case LMICE_SYSTEM_LIBUSB:
	    sysloaded = lmice_load("libusb.so");
	    break;
	default:
	    sysloaded = lmice_load("dummy.so");
	    break;
    }
    if (sysloaded) {
	lmice->count = mouse_plugin_init();
	return lmice->count;
    }
    ERR("Could not load plugin : %s", dlerror());
    return -1;
}

/** 
 * @brief freeing of the library resorses.
 * 
 * This function terminates all threads and free's up all memmory used by 
 * the library.
 * This function can be called multiple times.
 *
 * @return true on removal, false on error.
 */
int lmice_uninit()
{
    INIT_TEST(true);
    DBG("uninit()");
    lmice_timer_set(0);
    mouse_plugin_close();
    DBG("Freeing lmice");
    free(lmice);
    lmice = NULL;
    return true;
}
    
/** 
 * @brief Set timed reads interval
 *
 * This function sets the timer intervalls for the timed reads in 
 * microsecounds. Any value <= 0 stops the timedread thread.
 *
 * @param uwait	The number of microsecound the function should wait between
 *		each reading of the mices. 
 *		If <= 0 the thread will stop. if 0 it will wait for threadend. If
 *		lower than zero the function will not wait for thread to end
 *
 *		OBS! calling this function with < 0 as argument should only be
 *		done when the execution of the function needs to be fast. If called
 *		with -1 the pthread data will NOT be freed.
 *
 * @return true If the function exitet sucessfully (meaning the timed reads was
 *	    stopped, started or changed sucessfully. If this function returns 
 *	    false there is a major error in the threading system, beware!
 */
bool lmice_timer_set(int uwait)
{ 
    INIT_TEST(false);
    DBG("timer_set(uwait=%d)", uwait);
    /* Stop */
    if (uwait <= 0) {
       if (lmice->thread_read == 0 && lmice->uwait == 0)
	   return true; /* normal case when the system isn't running */
       if (lmice->thread_read != 0 && lmice->uwait != 0) {
	   

	   /* normal case when the system is running */
	   lmice->uwait = 0;
	   int data = 0;
	   if (uwait == 0) {
	       if (pthread_join(lmice->thread_read, (void *)data) != 0) {
		   ERR("Could not join read thread");
		   return false;
	       }
	   } else {
	       WRN("not waiting for threadend");
	   }
	   lmice->thread_read = 0;
	   return true;
       }
       /* if we come here, there is something wery wrong since :
	* uwait != 0 at the same time the thread is 0 or
	* uwait == 0 at the same timd the thread is (at oure knoleage) running
	*/
       ERR("STOP! TIMEDREADS THREAD AND UWAIT IS WRONG!");
       return false;
    }   
    /* Start */
    if (lmice->thread_read != 0 && lmice->uwait != 0) {
	lmice->uwait = uwait;
	return true; /* Normal case when the system is running */
    }
    if (lmice->thread_read == 0 && lmice->uwait == 0) {
	/* Thread is not running, lets start it */
	lmice->uwait = uwait;
	if (pthread_create(&lmice->thread_read, NULL, 
		    (void *)lmice_thread_read, NULL) != 0) {
	    ERR("Could not create read thread");
	    lmice->thread_read = 0;
	    lmice->uwait = 0;
	    return false;
	}
	return true;
    }
    /* if we come here, there is something wery wrong since :
    * ait != 0 at the same time the thread is 0 or
    * uwait == 0 at the same timd the thread is (at oure knoleage) running
    */
    ERR("START! TIMEDREADS THREAD AND UWAIT IS WRONG!");
    return false;
}
    
/** @brief Connect a callback function to a signal.
 *
 * Connects fcallback to signal. To remove a callback use NULL as fcallback 
 * argument.
 *
 * @param signal    The signal to connect to the callback,.
 * @param fcallback The callback function to connect to the signal, NULL do 
 *		    disconnect a signal.
 * @param data	    Data to send to the signal.
 *
 * @return true if the signal was setup, false if there was an error.
 */
int lmice_connect(LMiceSignal signal, LMiceCallback fcallback, void *data)
{
    INIT_TEST(false);
    DBG("connect(signal=%d)", signal);
    if (signal >= NUM_LMICE_CB) { 
	ERR("%d is not a defined signal", signal);
	return false;
    }
    lmice->callback[signal].function = fcallback;
    lmice->callback[signal].data = data;
    return true;
}
    
/**
 * The error message that the system generates if a library function is
 * called before lmice_init() that demands a initialized library.
 */
#define LMICE_ERROR_NOINIT "liblmice not initialized"

/** 
 * @brief Prints out the latest error
 *
 * This function prints out the last error encounterd by the library to 
 * the stderr stream. The error is cleard after it has been sent to stderr.
 */
void lmice_error_print()
{
    if (!lmice) {
	fprintf(stderr, "%s\n", LMICE_ERROR_NOINIT);
	fflush(stderr);
	return;
    } 
    fprintf(stderr, "%s\n", lmice->error);
    fflush(stderr);
    strcpy(lmice->error, "");
}

/** 
 * @brief Retrive the errorsting of the latest error
 *
 * Function returns the last error and clears the error.
 * 
 * @return A newly allocated string containing the error, NULL if there are no
 *	    errors to report.
 */
char *lmice_error_get()
{
    if (!lmice) {
	char *ret = malloc(strlen(LMICE_ERROR_NOINIT)+1);
	strcpy(ret, LMICE_ERROR_NOINIT);
	return ret;
    }
    if (lmice->error[0] == '\0')
	return NULL;
    char *ret = malloc(strlen(lmice->error)+1);
    strcpy(ret, lmice->error);
    strcpy(lmice->error, "");
    return ret;
}

/** 
 * @brief Read mouseinformation from connected mouses
 *
 * Reads the mices and stores the data in result, which is also returned.
 * 
 * This function may be used in two ways, either with continiously new 
 * retultstorages : 
 * 
 * <pre>
 * while (1) {
 *     LMiceResult *result = lmice_read(NULL, true);
 *      ... work with result ...
 *      lmice_result_delete(result);
 * }
 * </pre>
 * 
 * or with one signle resultstorage.
 * <pre>
 * LMiceResult *result = lmice_result_new();
 * while (1) {
 *     lmice_read(result, true);
 *     ... work with result ...
 * }
 * lmice_result_delete(result);
 * </pre>
 *
 * @param result    The pointer to which to store the results. If result is NULL a
 *		    new LMiceResult is created.
 * @param clear	    Set to true to clear internal counters on read, false will not
 *		    clear the internal counters.
 *
 * @return	    The pointer to result, if the parameter result is NULL a pointer
 *		    to a newly created LMiceResult is returned.
 */
LMiceResult *lmice_read(LMiceResult *result, bool clear)
{
    INIT_TEST(false);
    DBG("read()");
    if (!result)
	result = lmice_result_new();
    mouse_plugin_read_all(result, clear);
    return result;
}

/** 
 * @brief Clear the internal mouse counters
 */
void lmice_clear()
{
    if (!lmice)
	return;
    DBG("clear()");
    mouse_plugin_clear();
}

/** 
 * @brief get the number of mices connected to the system.
 *
 * Retrive the current number of mices connected to the system.
 *
 * return the number of mices, negative on error.
 */
int lmice_count()
{
    INIT_TEST(-1);
    DBG("count()");
    return lmice->count;
}

/**
 * @brief The default filename for configuration 
 */
#define  LMICE_DEF_CONFIG_FILE "liblmice.conf"

/** 
 * @brief load configuration from filename
 *
 * This function will try and load configuration from the supplied filename. If the filename
 * is NULL the function will use the LMICE_DEF_CONFIG_FILE as filename.
 *
 * @param filename the filename where configurations for the mice is stored, if NULL default file
 *		    will be used. 
 *
 * @return true on sucsess and false on error.
 */
bool lmice_config_load(char *filename) 
{
    INIT_TEST(false);
    DBG("config_load(filename=%s)",filename);
    if (!filename) {
	DBG("Using default filename %s", LMICE_DEF_CONFIG_FILE);
	filename = LMICE_DEF_CONFIG_FILE;
    }
    
    return mouse_plugin_config_load(filename);
}

/** @brief save configuration to filename
 *
 * This function will try and save configuration to the supplied filename. If the filename
 * is NULL the function will use the LMICE_DEF_CONFIG_FILE as filename.
 *
 * @param filename the filename where configurations for the mice will be stored, if NULL 
 *		    default file will be used. 
 *
 * @return true on sucsess and false on error.
 */
bool lmice_config_save(char *filename) 
{
    INIT_TEST(false);
    DBG("config_save(filename=%s)",filename);
    if (!filename) {
	DBG("Using default filename %s", LMICE_DEF_CONFIG_FILE);
	filename = LMICE_DEF_CONFIG_FILE;
    }
    return mouse_plugin_config_save(filename);
}

/** @brief use current resultset to configure the mice.
 *
 * This function will try and load configure the plugin from the supplied result. If result
 * is NULL the function will read the plugin and use that result to configure the plugin.
 *
 * @param result the result to use for configuration, if NULL a new reading will be used.
 *
 * @return true on sucsess and false on error.
 */
bool lmice_config_set(LMiceResult *result)
{
    INIT_TEST(false);
    DBG("config_set");
    if (!result) {
	result = lmice_result_new();
	if (!mouse_plugin_read_all(result, true)) {
	    lmice_result_delete(result);
	    ERR("Configuration result could not be loaded");
	    return false;
	}
	bool ret = mouse_plugin_config_set(result);
	lmice_result_delete(result);
	return ret;
    }
    return mouse_plugin_config_set(result);
}

/* @} */


