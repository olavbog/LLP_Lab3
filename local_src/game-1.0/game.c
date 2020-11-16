#define _GNU_SOURCE

#include <stdio.h> //printf
#include <linux/fb.h> //framebuffer
//declares uintx_t
#include <sys/mman.h> // adds mmap and ioctl functions
#include <unistd.h>  //adds close()
#include <sys/fcntl.h>  
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <signal.h> 
#include <unistd.h> 
#include <stdlib.h> 

#include "colors.h"
#include "function_def.h"

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

int init_gpio();
void sigio_handler(int);

int main(int argc, char *argv[]){

	fbfd = open("/dev/fb0",O_RDWR); //Open framebuffer driver
	ioctl(fbfd,FBIOGET_VSCREENINFO, &vinfo); //Get information about screen

	int framebuffer_size = vinfo.xres * vinfo.yres; //Size of the framebuffer
	fbp = mmap(NULL, framebuffer_size, PROT_READ | PROT_WRITE,MAP_SHARED, fbfd, 0);

	//Set white background
	for(int i = 0; i < framebuffer_size; i++)
		fbp[i] = WHITE;
	update_screen(0,0,vinfo.xres,vinfo.yres);
	display_string(SCREEN_WIDTH/2 -(13*8)/2, 1, "This is a dog", BLACK);

	init_gpio();

	while(1);
	//Unmap and close file
	close(gpio);
	munmap(fbp, framebuffer_size);
	close(fbfd);
	exit(EXIT_SUCCESS);
	return 0;
}


void update_screen(int x, int y, int width, int height){
	rect.dx = x;
	rect.dy = y;
	rect.width  = width;
	rect.height = height;
	ioctl(fbfd, 0x4680, &rect);
}

int init_gpio(){

	gpio = open("/dev/gamepad", O_RDWR);
	if(gpio<0){
		printf("Error opening gpio driver - %d\n",gpio);
		return -1;
	}
	if(signal(SIGIO,&sigio_handler) == SIG_ERR){
		printf("Error assigning handler to signal\n");
		return -1;
	}
	if(fcntl(gpio,F_SETOWN,getpid())== -1){
		printf("Error assigning owner\n");
		return -1;
	}
	long oflags = fcntl(gpio,F_GETFL);
	if(fcntl(gpio,F_SETFL, oflags|FASYNC)){
		printf("Error setting flags\n");
		return -1;
	}
	printf("Driver initialize success\n");
	return 0;
}


void sigio_handler(int no){
	printf("Hello from sigio_handler\n");
	return;
}