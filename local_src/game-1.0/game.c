#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h> 
#include <math.h>
#include <zlib.h>


#include "function_def.h"
#include "font8x8.h"

#define SCREEN_WIDHT  320
#define SCREEN_HEIGHT 240


#define BLOCKSIZE 10
uint32_t block[BLOCKSIZE*BLOCKSIZE];

int fbfd;
struct fb_copyarea rect;


uint16_t* fbp;
void draw_rect(int,int,int,int);
void draw_triangle(int,int,int);
void draw_pixel(int,int);
void display_string(int, int, char[]);
void display_char(int, int, char);
// void circle(int,int, int);


int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb0", O_RDWR); // O_RDWR = read and write access

	rect.dx = 0;
	rect.dy = 0;
	rect.width  = 320;
	rect.height = 240;


	printf("Driver test \n");

	int gpio = open("/dev/button_driver", O_RDWR);
	if(gpio != NULL)
	{
		printf(gpio);
		// gpio_p = mmap(NULL, 12, PROT_READ|PROT_WRITE,MAP_SHARED,GPIO, 0);
		uint8_t *buffer;

		read(gpio, buffer, 1);
		printf("\nGPIO read = ");
		printf(*buffer);
		printf("\n");
	}

	printf("Initializing game......\n");

	fbp = mmap(NULL,rect.width*rect.height*2,PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);




	printf("fbp worked well. Starting for loop\n");
	for(int i = 0; i < rect.width*rect.height*2; i++){
		fbp[i]=0x0DB7;
	}
	ioctl(fbfd, 0x4680, &rect);
	// draw_rect(50, 50, 20, 20);
	// draw_triangle(145,140,5);
	// draw_triangle(155,140,5);
	// draw_triangle(150,100,20);
	// draw_pixel(10,10);

	display_string(50, 50, "Hello World");
	printf("for loop finished.... \n");

	// Undo all opens and memory mappings
	munmap(fbp, SCREEN_WIDHT*SCREEN_HEIGHT*2);
	close(fbfd);

	printf("Exiting");
	exit(EXIT_SUCCESS);
}

void draw_rect(int x, int y, int width, int height){

	for(int row = y; row < height+y; row++)
	{
		for(int column = x; column < x+width; column++)
		{
			fbp[column + row*320]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
	printf("Drew rect");
}

void draw_triangle(int x,int y, int height){
	int width = 320;
	fbp[x + y*width]=0xFFFF;
	fbp[x + (y+height*2)*width]=0xFFFF;
	for(int row = 0; row < 2*height; row++)
	{
		for(int pos = -row-1; pos<row+1;pos++)
		{
			fbp[x+pos+(y+row)*width]=0xFFFF;
			fbp[x+pos+(y+4*height-row-2)*width]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
}


void draw_pixel(int x,int y)
{
	int width = 320;
	for(int j = 0; j<10;j++)
	{
		for(int i = x; i<x+10; i++)
		{
			fbp[i+(y+j)*width]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
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
	while(string[i])
	{
		/*
		   x coordinate will increase by 8 for the width of the char
		   and 1 for a space inbetween
		*/
		if(x+(i*9)+8+y*SCREEN_WIDHT > SCREEN_WIDHT*SCREEN_HEIGHT )
		{
			return;
		}
		display_char(x+(i*9),y, string[i]); 
		i++;
	}
	rect.dx = x;
	rect.dy = y;
	rect.width  = 8*i;
	rect.height = 8;
	// Update screen
	// Should only be the are of the text
	ioctl(fbfd, 0x4680, &rect);
}

void display_char(int x, int y, char ch)
{
	uint8_t ch_array[8] =  font8x8_basic[ch];
	for(int i = 0; i < 8; i++)
	{
		y += i*SCREEN_WIDHT;
		for(int j = 0; j < 8; j++)
		{
			if(font8x8_basic[ch][i]<<j & 1)
			{
				fbp[(x+j)+y*SCREEN_WIDHT]=0xFFFF;
			}
		}
	}
}