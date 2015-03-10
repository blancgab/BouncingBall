#ifndef _VGA_LED_H
#define _VGA_LED_H

#include <linux/ioctl.h>

#define X_MAX 1259
#define X_MIN 20
#define Y_MAX 459
#define Y_MIN 20

typedef struct {
  unsigned int x;  
  unsigned int y;
} vga_led_arg_t;

#define VGA_LED_MAGIC 'q'

/* ioctls and their arguments */
#define VGA_LED_DRAW_BALL _IOW(VGA_LED_MAGIC, 1, vga_led_arg_t *)

#endif
