#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h> 
#include <math.h>




#include "function_def.h"
#include "font8x8.h"
#include "colors.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


int fbfd;
struct fb_copyarea rect;
struct fb_var_screeninfo vinfo;


uint16_t* fbp;
int gpio;
// void draw_rect(int,int,int,int);
// void draw_triangle(int,int,int);
// void draw_pixel(int,int);
void display_string(int, int, char[]);
void display_char(int, int, char);
int gamepad();
int uninitialize_stuff();
void sigio_handler(int);




int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb0", O_RDWR); // O_RDWR = read and write access

	ioctl(fbfd,FBIOGET_VSCREENINFO,&vinfo);

	rect.dx = 0;
	rect.dy = 0;
	rect.width  = vinfo.xres;
	rect.height = vinfo.yres;

	int framebuffer_size = vinfo.xres * vinfo.yres;// * vinfo.bits_per_pixel/8;

	printf("width %d height %d bytesize %d", vinfo.xres, vinfo.yres, framebuffer_size);


	gamepad();
	printf("Initializing game......\n");


	//memory map the framebuffer so we can directly edit this and run the update
	//ioctl command
	fbp = (uint16_t *)mmap(NULL, framebuffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);




	printf("fbp worked well. Starting for loop\n");
	for(int i = 0; i < framebuffer_size; i++)
	{
		fbp[i]=0x0DB7;
	}
	// 

	printf("for loop finished.... \n");


	printf("Drawing string\n");
	display_string(SCREEN_WIDTH/2 -(13*8)/2, 1, "This is a dog");
	
	rect.dx = 0;
	rect.dy = 0;
	rect.width  = vinfo.xres;
	rect.height = vinfo.yres;


	ioctl(fbfd, 0x4680, &rect);
	while(1);
	// Undo all opens and memory mappings
	printf("munmap\n");
	munmap(fbp, framebuffer_size);

	printf("close\n");
	close(fbfd);
	printf("Exiting\n");
	exit(EXIT_SUCCESS);
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
	rect.dx = x;
	rect.dy = y;
	rect.width  = 8*i;
	rect.height = 9;
	// Should only be the square of the text
	// ioctl(fbfd, 0x4680, &rect);
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

int gamepad()
{
	printf("Driver test \n");

	gpio = open("/dev/gamepad", O_RDWR);
	if(gpio == -1)
	{
		// Couldnt open file
		return -1;	
	}
	// gpio_p = mmap(NULL, 12, PROT_READ|PROT_WRITE,MAP_SHARED,GPIO, 0);
	if(signal(SIGIO,&sigio_handler) == SIG_ERR)
	{
		// Error for registering signal handler
		return -1;
	}
	if (fcntl(fileno(device),F_SETOWN,getpid()) == -1)
	{
		// error setting owner
		return -1;
	}
	long oflags = fcntl(fileno(device),F_GETFL);
	if(fcntl(fileno(device),F_SETFL, oflags|FASYNC) == -1)
	{
		//Error setting flag
		return -1;
	}

	return 0
	// uint8_t *buffer;

	// read(gpio, buffer, 1);
	// printf("GPIO read = %x",*buffer);
	// printf("\n");
	
}

int uninitialize_stuff()
{
	close(gpio);
}
void sigio_handler(int signo)
{
	printf("Interrupt has occured number something %d", signo);
}