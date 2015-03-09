/*
 * Userspace program that communicates with the led_vga device driver
 * primarily through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

#include <stdio.h>
#include "vga_led.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

void write_position(int, int);

int vga_led_fd;

int main()
{
  vga_led_arg_t vla;
  int i;
  static const char filename[] = "/dev/vga_led";

  int x_pos, x_vel, y_pos, y_vel;

  /* Initial Position and Velocity Vectors */
  x_pos = 200;
  y_pos = 200;
  x_vel = 1;
  y_vel = 1;

  printf("VGA LED Userspace program started\n");

  if ((vga_led_fd = open(filename, O_RDWR)) == -1) 
  {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  while (1)
  {
    x_pos = x_pos + x_vel;
    y_pos = y_pos + y_vel;
    
    if (x_pos > X_MAX)
    {
      x_pos = X_MAX;
      x_vel = -x_vel;
    }
    else if (x_pos < X_MIN)
    {
      x_pos = X_MIN;
      x_vel = -x_vel;
    }
    
    if (y_pos > Y_MAX)
    {
      y_pos = Y_MAX;
      y_vel = -y_vel;
    }
    else if (y < Y_MIN)
    {
      y_pos = Y_MIN;
      y_vel = -y_vel;
    }

    write_position(x_pos, y_pos);
    usleep(400000);    
  }  

  printf("VGA LED Userspace program terminating\n");
  return 0;
}

void write_position(int x, int y)
{
  vga_led_arg_t vla;  
  printf("BALL POSITION (%d, %d)\n", x, y);
  vla.x = (u16) x;
  vla.y = (u16) y;

  if (ioctl(vga_led_fd, VGA_LED_DRAW_BALL, &vla))
  {
    perror("ioctl(VGA_LED_DRAW_BALL) failed");
    return;
  }
}
