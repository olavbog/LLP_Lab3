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

#define BLOCKSIZE 10
uint32_t block[BLOCKSIZE*BLOCKSIZE];

int fbfd;
struct fb_copyarea rect;
struct psf_header {
	uint8_t magic[2];
	uint8_t filemode;
	uint8_t fontheight;
};

uint16_t* fbp;
void draw_rect(int,int,int,int);
void draw_triangle(int,int,int);
void draw_pixel(int,int);
void display_string(char s[], int x, int y, uint8_t chars[]);
void display_char(char ch, int x, int y, uint8_t chars[]);
void set_block(uint32_t x, uint32_t y, uint32_t L);
// void circle(int,int, int);


int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb0", O_RDWR); // O_RDWR = read and write access

	rect.dx = 0;
	rect.dy = 0;
	rect.width = 320;
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

	display_string("Hello World", 50,50)
	ioctl(fbfd, 0x4680, &rect);

	printf("for loop finished.... \n");
    // close(fbfd);

	printf("Exiting");

	// exit(EXIT_SUCCESS);
	// exit(0);
	return EXIT_SUCCESS;
	// MCU doesnt return back to console here.. Why??
}

void draw_rect(int x, int y, int width, int height){

	for(int row = y; row < height+y; row++){
		for(int column = x; column < x+width; column++){
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
	for(int row = 0; row < 2*height; row++){
		for(int pos = -row-1; pos<row+1;pos++){
			fbp[x+pos+(y+row)*width]=0xFFFF;
			fbp[x+pos+(y+4*height-row-2)*width]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
}


void draw_pixel(int x,int y)
{
	int width = 320;
	for(int j = 0; j<10;j++){
		for(int i = x; i<x+10; i++)
			fbp[i+(y+j)*width]=0xFFFF;
	}
	ioctl(fbfd, 0x4680, &rect);
}
void set_block(uint32_t x, uint32_t y, uint32_t L)
{
	for(int i = 0; i < L; i++){
		for(int j = 0; j < L; j++)
			fbp[i]=0xF81B;
	}
}

void display_char(char ch, int x, int y, uint8_t chars[])
{
	uint8_t row;
	int x1 = x;
	for(int i = 0; i < header.fontheight;i++){
		row = chars[ch*header.fontheight+i];
		for(int j = 0< j<8; j++){
			if(row &0x80){
				set_block(x1,y,BLOCKSIZE);
			}
			row = row << 1;
			x1 = x1 + BLOCKSIZE;
		} 
		y = y + BLOCKSIZE;
		x1 = x;
	}
}

void display_string(char s[], int x, int y)
{
	gzFile font = gzopen("/pathtofile",'r');

	gzread(font, &header, sizeof(header));
	uint8_t chars[header.fontheight * 256];
	gzread(font, chars,header.fontheight*256);

	int i = 0;
	while(s[i])
	{
		display_char(s[i],x,y,chars);
		x = x + BLOCKSIZE*9;
		i++;
	}

}