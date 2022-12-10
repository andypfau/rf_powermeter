#ifndef USB_H
#define	USB_H


#include <stdint.h>
#include <stdbool.h>


void usb_init(void);
void usb_loop(void);

bool usb_ready_to_send(void);
void usb_set_data(char *buffer, int size);
int usb_get_data(char *buffer, int max_size);


#endif	/* USB_H */
