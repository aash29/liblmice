
#ifndef _LIBLMICE_H
#define _LIBLMICE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @mainpage liblmice index page
 *
 * <H2>Public interface</H2>
 * 
 * @ref liblmice
 *
 * @ref lmiceresult
 *
 *
 * <H2>Internal interfaces</H2>
 * 
 * @ref intlmice
 *
 * @ref intresdata
 *
 * @ref complugin
 *
 * @ref dummyplugin
 *
 * @ref devfsplugin
 *
 * @ref libusbplugin
 *
 */


/** @ingroup liblmice
 * definition of true is that it is not zero
 */
#ifndef true
    #define true 42
#endif
/** @ingroup liblmice
 * definition of false is that it is zero
 */
#ifndef false
    #define false 0
#endif

/** @ingroup liblmice
 * definition of a bool, it's an int :)
 */
#ifndef bool
    #define bool int
#endif

/** @ingroup liblmice
 * The default path to read when initilazing 
 */
#define LMICE_DEV_PATH "/dev/input/"

/** @ingroup liblmice
 * The filter string. Only files matching this string will be initialized 
 */
#define LMICE_DEV_FILTER "mouse"

/** @ingroup lmiceresult
 * @brief General information for the return.
 *
 * Actual data. This is the permouse data sent from the library
 * It is included in a LMiceResult
 */
typedef struct {
    /* raw data */
    int reads;	    /**< current number of reads */
    int raw_x;	    /**< raw movement in x, 
		      * this is the real mousesensor movement */
    int raw_y;	    /**< raw movement in y, 
		      * this is the real mousesensor movement */
    int overflow_x; /**< overflows in x */
    int overflow_y; /**< overflows in y */
    /* configuration values */
    float move_x;   /**< movement in x, this is the calculated value */
    float move_y;   /**< movement in y, this is the calculated value */
} LMiceData;

/** @ingroup liblmice
 * Maximum number of mice, 127 is USB max so 128 should do it
 */
#define LMICE_MAX_MICE 128

/** @ingroup lmiceresult
 * @brief The basic resultset
 *
 * This is the main structure used in communication with the calling 
 * program.
 */
typedef struct {
    int count; /**< the actual number of mices in mouse variable */
    unsigned long long time; /**< time between the reads */
    LMiceData *avg; /**< the avarage of the information */
    LMiceData *data[LMICE_MAX_MICE]; /**< the information from each mouse */
} LMiceResult;

/** @ingroup liblmice
 * @brief Signaltypes one can connect to.
 *
 * These are the diferent signals that have connectable callbacks.
 * Use lmice_connect to connect a signal.
 */
typedef enum {
    LMICE_CB_NEW_DATA,	/** Emits a signal when there is data in the system.
			  * Callback function is:
			  *
			  * <pre>int function_name(LMiceSignal signal, int mouse_count, LMiceResult *lmice_result, void *data);</pre>
			  */
    LMICE_CB_OVERFLOW,	/** When a overflow in a mouse is detected. 
			  * Callback function is:
			  * 
			  * <pre>int function_name(LMiceSignal signal, int mouse_number, LMiceResult *mouse_result, void *data);</pre>
			  */
    NUM_LMICE_CB    
} LMiceSignal;


/** @ingroup liblmice
 * @brief Set the system type to load
 * 
 * One of these enums should be used with lmice_init(), they decide what plugin the library should
 * use.
 */
typedef enum {
    LMICE_SYSTEM_DUMMY, /**< Usa a dummy plugin. */
    LMICE_SYSTEM_DEVFS, /**< Use the devfs system, 
			  this let's the kernel do the mousereadings and the 
			  library reads the mice using /dev/input */
    LMICE_SYSTEM_LIBUSB, /**< Use the libusb library.
			   the library read direcly the usb data from the kernel. 
			   No conversion is made in the kernel, infact the library
			   must frist detach the usb devices so the mice will stop
			   working in X for example.
			   */
    NUM_LMICE_SYSTEMS	/**< number of implemented systems */
} LMiceSystemType;

/** @ingroup liblmice
 * @brief Definition of the callback function 
 *
 * All callbackfunctions need to correspont to this callback.
 * A callback should have this system:
 *
 * int (*function_name)(LMiceSignal signal, int sigspecint,
 *	LMiceResult *result, void *data)
 */
typedef int (*LMiceCallback)(LMiceSignal,int, LMiceResult*, void*);


/* exported functions in lmice.c */

/* initializaton */
int lmice_init(LMiceSystemType system);
int lmice_uninit();

/* Standard features */
LMiceResult *lmice_read(LMiceResult *result, bool clear);
int lmice_count();
void lmice_clear();
int lmice_connect(LMiceSignal signal, LMiceCallback callback, void *data);

/* Configurations */
bool lmice_config_set(LMiceResult *result);
bool lmice_config_save(char *filename);
bool lmice_config_load(char *filename);

/* Timing and callbacks */
bool lmice_timer_set(int usec);
int lmice_timmer_get();

/* Error handling */
void lmice_error_print();
char *lmice_error_get();

/* exported functions in result.c */
/* Result handling */
LMiceResult *lmice_result_new(); 
int lmice_result_clear(LMiceResult *result); 
LMiceResult *lmice_result_dup(LMiceResult *src);
LMiceResult *lmice_result_copy(LMiceResult *dst, LMiceResult *src);
int lmice_result_delete(LMiceResult *result); 

#ifdef __cplusplus
}
#endif

#endif

