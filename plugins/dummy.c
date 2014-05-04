
#include "plmouse.h"
#include "../result.h"
#include <stdio.h>
#include <stdlib.h>

#define DBG_FILE "dummy.c"

/**@defgroup dummyplugin The dummy plugin.
 * @ingroup complugin
 * 
 * This plugin will create three "viritaul" mouses that never move.
 *
 * @{
 */

/**
 * @brief Initialization of dummy plugin.
 * 
 * This initializes three viritual dummy mouses
 *
 * @return always returns 3.
 */
int mouse_pl_init()
{
    DBG("pl_init()");
    if (mouse_count > 0)
	return mouse_count;
    int i=0;
    for (;i<3;i++) {
	mouse[i] = malloc(sizeof(struct LMice));
	mouse[i]->system = NULL;
	mouse[i]->scale = 0;
	mouse[i]->forward = 1;
	mouse[i]->data = lmice_data_new();
	mouse[i]->filename = NULL;
	lmice_data_clear(mouse[i]->data);
    }
    mouse_count = 3;
    return 3;
}

/**
 * @brief close plugin
 *
 * Frees the "viritual" mouses.
 */
void mouse_pl_close()
{   
    DBG("pl_close()");
    int i=0;
    LOCK();
    for (;i < 3; i++) {
	lmice_data_delete(mouse[i]->data);
	free(mouse[i]);
    }
    mouse_count = 0;
    UNLOCK();
    DBG("pl_close()");
    printf("Close\n");
    
}

/* @} */
