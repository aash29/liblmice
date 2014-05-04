
#ifndef _PLMOUSE_H
#define _PLMOUSE_H

#include "../liblmice.h"
#include "../lmice.h"
#include "../result.h"


/**@ingroup complugin
 *
 * Basic common structures for plugins 
 *
 * */
struct LMouse {
    void *system; /**< system specific data */
    char *filename; /**< the devicename */
    LMiceData *data; /**< current readings */
    float scale; /**< scaling of the mosue */
    float forward; /**< forward direction of the mosue in radians */
};

/**@ingroup complugin
 *
 * the main scale of the mouse, this scaling factor is used for all the mice 
 */
float main_scale;

#include <math.h>
/**@ingroup complugin
 *
 * Now I always beleved that PI was exactly tree, but I seem to have been mistacen... 
 */
#ifndef PI
    #define PI 3.1416 
#endif

/**@ingroup complugin
 *
 * The internal mouse structures 
 */
struct LMouse *mouse[LMICE_MAX_MICE];


/**@ingroup complugin
 *
 * set to false on read_all, set to true whenever... 
 */
bool mouse_has_data;

/**@ingroup complugin
 *
 * The current number of mice used in the system 
 */
extern int mouse_count;

/**@ingroup complugin
 *
 * The internal result mutex used for locking 
 */
pthread_mutex_t main_mutex; 

/**@ingroup complugin
 *
 * Locks the internal result for read/write access 
 */
#define LOCK()  pthread_mutex_lock(&main_mutex)

/**@ingroup complugin
 *
 * Unlocks the internal result 
 */
#define UNLOCK()  pthread_mutex_unlock(&main_mutex)

/* To be used directly in plugin */
int mouse_pl_init();

void mouse_pl_close();

/* Common */
void mouse_pl_preinit();

int mouse_pl_read_all(LMiceResult *result, bool clear);

int mouse_pl_read(int mouse, LMiceData *data);

bool mouse_pl_new_data();

bool mouse_pl_config_load(char *filename);

bool mouse_pl_config_save(char *filename);

bool mouse_pl_config_set(LMiceResult *result);

void mouse_pl_clear();


#endif


