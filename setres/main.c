#include <stdio.h>
#include <stdlib.h>
#include <usb.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
	printf("\nUse %s [0|400|800]\n\n", argv[0]);
        return 0;
    }
    printf("\n");
    int res = atoi(argv[1]);
    char setres = 0;
    if (res != 400 && res != 800 && res != 0) {
	printf("\nArgument 1 must be 400 or 800 to set the devices or 0 to read them.\n\n");
	return 0;
    }
    if (res == 800)
	setres = 0x04;
    else 
	setres = 0x03;
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_init();
    usb_find_busses();
    usb_find_devices();
    int i=0;
    char msg[4];
    for (bus = usb_busses; bus; bus = bus->next) {
	for (dev = bus->devices; dev; dev = dev->next) {
	    if (dev->descriptor.idVendor != 0x046D)//  ||   dev->descriptor.idProduct != 0xC01B)
		continue;
	    usb_dev_handle *usb_h;
	    usb_h = usb_open(dev);
	    if (res == 0) {
		printf("Reading movement\n");
		usb_control_msg(usb_h, 
			0xa1, 
			0x01, 
			0x0100,
			0x0000,
			msg,
			0x0004,
			10);
		printf("reading is %x %x %x %x\n", msg[0], msg[1], msg[2], msg[3]);
		printf("Reading resolution on mouse %d, ", i);
		usb_control_msg(usb_h, 
		    USB_TYPE_VENDOR | USB_ENDPOINT_IN, 
		    0x01, 
		    0x000E, 
		    0x0000, 
		    &setres, 
		    0x0001, 
		    100);
		if (setres == 0x04)
		    printf("it reads 800\n");
		else if (setres == 0x03)
		    printf("it reads 400\n");
		else
		    printf("it reads to a false value 0x%04x\n", setres);
		
	    } else {
		printf("Setting resolution on mouse %d to %d\n", i, res);
		usb_control_msg(usb_h, USB_TYPE_VENDOR, 0x02, 0x000E, setres,
		                         NULL,  0x0000, 100);
	    }	
	    usb_close(usb_h);
	    i++;
	}
    }
    printf("\n");
			
    return 0;
}
