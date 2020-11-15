#include <linux/fb.h>
#include <stdint.h> //declares uintx_t

#include "font8x8.h"
#include "colors.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

/*Framebuffer variables*/

int fbfd; 						//File open
struct fb_copyarea rect; 		//Defines the area that is updated in the framebuffer
struct fb_var_screeninfo vinfo; //Can receive info about the screen - resolution
uint16_t* fbp; 					//Memory mapping - 16 bits per pixel on the screen

/*GPIO variables*/
int gpio; //File open

/*Function declarations*/
void display_string(int, int, char[]);
void display_char(int, int, char);
int gamepad();
int uninitialize_stuff();
void sigio_handler(int);

int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb0",O_RDWR); //Open framebuffer driver
	ioctl(fbfd,FBIOGET_VSCREENINFO, &vinfo); //Get information about screen

	int framebuffer_size = vinfo.xres * vinfo.yres; //Size of the framebuffer
	fbp = mmap(NULL, framebuffer_size, PROT_READ | PROT_WRITE,MAP_SHARED, fbfd, 0);

	for(int i = 0; i < framebuffer_size; i++)
		fbp[i] = WHITE;

	rect.dx = 0;
	rect.dy = 0;
	rect.width  = vinfo.xres;
	rect.height = vinfo.yres;
	ioctl(fbfd, 0x4680, &rect);

	return 0;
}
