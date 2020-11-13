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

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240


#define BLOCKSIZE 10
uint32_t block[BLOCKSIZE*BLOCKSIZE];

int fbfd;
struct fb_copyarea rect;


uint16_t* fbp;
// void draw_rect(int,int,int,int);
// void draw_triangle(int,int,int);
// void draw_pixel(int,int);
void display_string(int, int, char[]);
void display_char(int, int, char);




int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb0", O_RDWR); // O_RDWR = read and write access

	rect.dx = 0;
	rect.dy = 0;
	rect.width  = 320;
	rect.height = 240;


	printf("Driver test \n");

	int gpio = open("/dev/button_driver", O_RDWR);
	if(gpio != -1)
	{
		printf("GPIO %i",gpio);
		// gpio_p = mmap(NULL, 12, PROT_READ|PROT_WRITE,MAP_SHARED,GPIO, 0);
		uint8_t *buffer;

		read(gpio, buffer, 1);
		printf("\nGPIO read = ");
		printf("GPIO read = %x",*buffer);
		printf("\n");
	}

	printf("Initializing game......\n");


	//memory map the framebuffer so we can directly edit this and run the update
	//ioctl command
	fbp = mmap(NULL, SCREEN_WIDTH*SCREEN_HEIGHT*2, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);




	printf("fbp worked well. Starting for loop\n");
	for(int i = 0; i < rect.width*rect.height*2; i++)
	{
		fbp[i]=0x0DB7;
	}
	ioctl(fbfd, 0x4680, &rect);

	printf("for loop finished.... \n");


	printf("Drawing string\n");
	display_string(1, SCREEN_WIDTH/2, "This is a dog");
	

	// Undo all opens and memory mappings
	printf("munmap\n");
	munmap(fbp, SCREEN_WIDTH*SCREEN_HEIGHT*2);

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
	ioctl(fbfd, 0x4680, &rect);
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