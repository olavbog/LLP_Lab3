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

	display_string(SCREEN_WIDTH/2 -(13*8)/2, 1, "This is a dog");

	//Unmap and close file
	munmap(fbp, framebuffer_size);
	close(fbfd);

	return 0;
}

void display_char(int x, int y, char ch)
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if((font8x8_basic[ch][i]>>j) & 1)
			{
				fbp[(x+j)+(i+y)*SCREEN_WIDTH]=0xFFFF;
			}
		}
	}
}

void display_string(int x, int y, char string[])
{
	/*
		Loops through all chars in the input strings
		and writes it to the LCD screen at the given
		coordinates
	*/
	int i = 0;
	//loop through all chars in input string
	printf("display_string function\n");
	while(string[i])
	{
		/*
		   x coordinate will increase by 8 for the width of the char
		   and 1 for a space inbetween
		*/
		if(x+(i*8)+8+y*SCREEN_WIDTH > SCREEN_WIDTH*SCREEN_HEIGHT )
		{
			printf("Word outside of screen range");
			return;
		}
		display_char(x+(i*8),y, string[i]); 
		i++;
	}
	i--;
	rect.dx = x;
	rect.dy = y;
	rect.width  = 8*i;
	rect.height = 9;
	// Should only be the square of the text
	ioctl(fbfd, 0x4680, &rect);
}
