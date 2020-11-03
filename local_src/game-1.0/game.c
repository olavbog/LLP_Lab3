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

int fbfd;
struct fb_copyarea rect;

uint16_t* fbp;
void draw_rect(int,int,int,int);
void draw_triangle(int,int,int);
void draw_pixel(int,int);
// void circle(int,int, int);


int main(int argc, char *argv[])
{
	fbfd = open("/dev/fb0", O_RDWR); // O_RDWR = read and write access

	rect.dx = 0;
	rect.dy = 0;
	rect.width = 320;
	rect.height = 240;


	printf("Initializing game......\n");

	fbp = mmap(NULL,rect.width*rect.height*2,PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);





	printf("fbp worked well. Starting for loop");
	for(int i = 0; i < rect.width*rect.height*2; i++){
		fbp[i]=0x0DB7;
	}
	ioctl(fbfd, 0x4680, &rect);
	draw_rect(50, 50, 20, 20);
	draw_triangle(145,140,5);
	draw_triangle(155,140,5);
	draw_triangle(150,100,20);
	draw_pixel(10,10);

	printf("for loop finished....");
    // close(fbfd);

	printf("Exiting");

	exit(EXIT_SUCCESS);
	return EXIT_SUCCESS;
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


void draw_pixel(int x,int y){
	int width = 320;
	for(int j = 0; j<10;j++){
		for(int i = x; i<x+10; i++)
			fbp[i+(y+j)*width]=0xFFFF;
	}
	ioctl(fbfd, 0x4680, &rect);
}

// void circle(int x,int y, int r){
// 	int width = 320;
// 	int r2 = r*r;
// 	fbp[x+(y+r)*width]=0xFFFF;
// 	fbp[x+(y+r)*width]=0xFFFF;
// 	for(int i = 1; i< r; i++){
// 		int j = pow(r2-i*i,0.5)+0.5; 
// 	}
// }