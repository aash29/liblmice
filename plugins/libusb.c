#define _MULTI_THREADED
#include "plmouse.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <usb.h>
#include <pthread.h>
#include <string.h>

#define DBG_FILE "libusb.c"

#define LIBUSB_FORCE_OPEN true

/**@defgroup libusbplugin The libusb plug-in specifics
 * @ingroup complugin
 *
 * Functions and variable specific to the usage of libusb in this plug-in
 * The libusb plug-in starts a theread for each mouse found and polls
 * the mouse at minimal alowed interval. 
 * 
 * @{
 */

/**
 * @brief The libusb system items.
 */
struct libUsbItem {
    usb_dev_handle *usb_h; /**< the usbhandle currently used */
    pthread_t thread; /**< the threadhandle */
};


struct LMouse *mouse_inits(struct usb_device *dev);

/** 
 * @brief true if threads should looprun, false if they shold exit 
 */
bool thread_run;

/**
 * @brief thread that reads usb information
 *
 * This thrad reads the current status of a mouse and saves the data in 
 * it's structure.
 *
 * @param data	a pointer to the LMouse strucure to use
 *
 * @return disregarded and not used (pthread specific)
 */
void *mouse_threads(void *data)
{
    DBG("mouse_thread()");
    struct LMouse *mouse = (struct LMouse *) data;
    struct libUsbItem *system = (struct libUsbItem *) mouse->system;
    signed char msg[4] = {0x00, 0x00, 0x00, 0x00};
    signed char ovf;
    while (thread_run) {
	if (usb_control_msg(system->usb_h,
		0xa1,
		0x01,
		0x0100,
		0x0000,
		msg,
		0x0004,
		10) == 4) {
	
	    if (msg[1] || msg[2]) {
		LOCK();
		mouse->data->raw_x += msg[1];
		mouse->data->raw_y += msg[2];
		mouse->data->reads++;
		ovf = msg[1];
		if (ovf < 0)
		    ovf = -ovf;
		if (ovf == 0x7F) {
		    ERR("Overflow on X %02x", msg[1]);
		    mouse->data->overflow_x++;
		}
		ovf = msg[2];
		if (ovf < 0)
		    ovf = -ovf;
		if (ovf == 0x7F) {
		    ERR("Overflow on X %02x", msg[2]);
		    mouse->data->overflow_y++;
		}
		    
		UNLOCK();
		mouse_has_data = true;
	    }
	}
    }	
    return NULL;
}

/**
 * @brief Initialize the libusb structure and start reading threads.
 *
 * This function starts the libusb structure and finds apropriate mouse
 * to read from. 
 *
 * @return the number of mice found or a negative value on error
 */
int mouse_pl_init()
{
    struct usb_bus *bus = NULL;
    struct usb_device *dev = NULL;
    usb_init();
    usb_find_busses();
    usb_find_devices();
    bus = usb_get_busses();
    int cnt = 0;
    thread_run = true;
    LOCK();
    for (bus = usb_busses; bus; bus = bus->next) {
	for (dev = bus->devices; dev; dev = dev->next) {
	    if (dev->descriptor.idVendor != 0x046D)
		continue;
	    mouse[cnt] = mouse_inits(dev);
	    DBG("Created thread %d", cnt);
	    if (mouse[cnt])
		cnt++;
	}
    }	    
    mouse_count = cnt;
    UNLOCK();
    return cnt;
}

/**
 * @brief Initialize a mouse and strart it's thread
 *
 * @param dev the usb_device to be initialized
 *
 * @return the newly created LMouse structure, or NULL on failur.
 */
struct LMouse *mouse_inits(struct usb_device *dev)
{
    DBG("mouse_inits()");
    struct LMouse *mouse = malloc(sizeof(struct LMouse));
    //strcpy(mouse->filename, dev->descriptor.Serial);
    mouse->forward = 0;
    mouse->scale = 1;

    struct libUsbItem *system = malloc(sizeof(struct libUsbItem));
    mouse->system = system;
    system->usb_h = usb_open(dev);
#if LIBUSB_FORCE_OPEN == true
    usb_detach_kernel_driver_np(system->usb_h, 0);
#endif
    if (!system->usb_h) {
	ERR("Could not open mouse");
	free(system);
	free(mouse);
	return NULL;
    }
    mouse->data = lmice_data_new();
    int len = 16;
    char* buffer = (char*)malloc(len);
    usb_get_string_simple(system->usb_h, dev->descriptor.iSerialNumber,
	        buffer, len);
    if (strlen(buffer) > 4)
	strcpy(mouse->filename, buffer);
    else
	mouse->filename = NULL;
    free(buffer);
    
    lmice_data_clear(mouse->data);
    system->thread = 0;
    if (pthread_create(&(system->thread), NULL, (void *)mouse_threads, (void *)mouse) != 0) {
	WRN("Thread not created!!" );
	system->thread = 0;
    }
    return mouse;
}

/** 
 * @brief free data used by the plugin and stops all threads.
 */
void mouse_pl_close()
{
    thread_run = false;
    int i=0;
    LOCK();
    for (;i<mouse_count;i++) {
	struct libUsbItem *system = (struct libUsbItem *) mouse[i]->system;
	void *data = malloc(4);
	if (system->thread) {
	    DBG("Remove thread %d", i);
	    pthread_join(system->thread, data);

	    usb_close(system->usb_h);
	    DBG("Thread removed");
	}
	free(data);
	free(system);
	lmice_data_delete(mouse[i]->data);
	free(mouse[i]);
    }
    mouse_count = 0;
    UNLOCK();
}

/* @} */
