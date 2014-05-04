
#ifndef _LMICE_H
#define _LMICE_H

#include "liblmice.h"
//#include "mouse.h"
#include <stdio.h>
#include <pthread.h>



/*
 * Set debug using the make system like this "make DEBUG=true all" when compiling
 */
#ifdef DEBUG
    /* for explenations se else statement */
    #define DEBUG_PRINT(x, y...) \
	fprintf(stderr, "%s LIBLMICE:%s:",x, DBG_FILE); \
	fprintf(stderr, y); \
	fprintf(stderr, "\n"); \
	fflush(stderr)
    
    #define DBG(x...) DEBUG_PRINT("   ", x)

    #define WRN(x...) DEBUG_PRINT("www", x)

    #define ERR(x...) DEBUG_PRINT("EEE", x); \
	    sprintf(lmice->error, x)
#else
    /**@ingroup intlmice
     * @brief Degug info
     * 
     * printf(x) basicaly, altho does linefead. 
     * This is used for basic debugging. 
     * output comes to stderr. Not used if DEBUG is set to 0
     */
    #define DBG(x...) 
    /**@ingroup intlmice
     * @brief Debug warn information
     * 
     * like DBG but produces www at the start of the line to stderr.
     * Not used if DEBUG is set to 0
     */
    #define WRN(x...)
    /**@ingroup intlmice
     * @brief Set error message and debug error output
     *
     * like DGB but produces EEE at the start of the line to stderr. 
     * This function also setts the error text for lmice_error_* functons. 
     * Only setts error text if DEBUG is set to 0
     */
    #define ERR(x...) sprintf(lmice->error, x);
#endif

/**@ingroup intlmice
 * Removes "not used" warnings for callbackfunctions. Is optimized away later 
 * so no extra code is produced (I think :) )
 */
#define REMOVE_CC_WARNING_ON(x) ((x) = (x))


/** @ingroup intlmice
 * @brief Storage of callbackfunctions 
 */
struct Callback {
    LMiceCallback function; /**< function pointer, NULL if there are nong */
    void *data;		    /**< unhandled data from lmice_connect */
};

/**@ingroup intlmice
 * @brief thread commands
 * 
 * Commands that can be sent to poll thread.
 */
enum {
    NO_CMD,	    /**< No command, normal state. */
    EXIT_THREAD,    /**< Exit the thread freeing data and such.. */
};


/** @ingroup intlmice
 * @brief The internal structure of the liblmice library
 */
struct LMice {
    LMiceSystemType system; /**< the system used to read the mice. */
    void *plhandle;	    /**< handle for the plugins */
    char error[80];	    /**< Latest error message */
    int count;		    /**< The number of mice currently connected to the 
			      * system */
    pthread_t thread_read;  /**< Thread the mouse reads are running in */
    int uwait;		    /**< waiting time in microsecounds for 
			      * readthread */
    float scale;	    /**< main scale, 
			      * this will be applied to all mice. */
    struct Callback callback[NUM_LMICE_CB]; /**< callbackfunctions */
};

/**@ingroup intlmice
 * @brief Test if library is initialized.
 *
 * a INIT_TEST(returnvalue) should be placed in the beginning of every 
 * public function.
 */
#define INIT_TEST(x) if (!lmice) return (x);

/**
 * @ingroup intlmice
 * External decleration of lmice to make it avalible throuout the lib.
 */
extern struct LMice *lmice;

/**
 * @ingroup intlmice
 * function like call to determine if a callback is connected to the signal x
 * 
 * @param x signal to test
 *
 * @return 1 on connected callback, else 0 (false). OBS! 1 != true 
 */
#define HAS_CALLBACK(x) (lmice->callback[(x)].function != NULL)
int lmice_callback(LMiceSignal signal, int ival);
	    
#endif
