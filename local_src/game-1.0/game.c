#define _GNU_SOURCE

#include <stdio.h> //printf
#include <linux/fb.h> //framebuffer
//declares uintx_t
#include <sys/mman.h> // adds mmap and ioctl functions
#include <unistd.h>  //adds close() and sleep() functions
#include <sys/fcntl.h>  
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <signal.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#include "colors.h"
#include "function_def.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SELECTED     1
#define NOT_SELECTED 0

#define FRONTPAGE 0
#define GAMEPAGE  1


/*Framebuffer variables*/
int fbfd; 						//File open
struct fb_var_screeninfo vinfo; //Can receive info about the screen - resolution
uint16_t* fbp; 					//Memory mapping - 16 bits per pixel on the screen
struct Game_character square_box;
int framebuffer_size;


enum direction{UP,DOWN,RIGHT,LEFT,ENTER} dir;


/*GPIO variables*/
int gpio; //File open

struct Game_frontscreen frontscreen = {
	 FRONTPAGE,
	 3, //number of links on the frontscreen
	 0,
	{
		{SCREEN_WIDTH/2 - 8/2*8, SCREEN_HEIGHT/3+00, "New Game",8*8,SELECTED, 0},
		{SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3+10, "Highscores",10*8, NOT_SELECTED,1},
		{SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3+20, "Exit Game",9*8, NOT_SELECTED,2},
	}
};
struct screens curr_screen;
struct Game_screen game_screen = {
	GAMEPAGE
};




/*Function declarations*/

int init_gpio();
void sigio_handler(int);
void start_screen();
void start_game();
void selected_background(int,int,int,int);

int main(int argc, char *argv[]){

	fbfd = open("/dev/fb0",O_RDWR); //Open framebuffer driver
	ioctl(fbfd,FBIOGET_VSCREENINFO, &vinfo); //Get information about screen

	framebuffer_size = vinfo.xres*vinfo.yres;//vinfo.smem_len; //Size of the framebuffer
	fbp = mmap(NULL, framebuffer_size, PROT_READ | PROT_WRITE,MAP_SHARED, fbfd, 0);

	//Set white background
	for(int i = 0; i < framebuffer_size; i++)
		fbp[i] = BLACK;
	update_screen(0,0,vinfo.xres,vinfo.yres);
	display_string(SCREEN_WIDTH/2 -(13*8)/2, 1, "This is a dog",13, WHITE);

	init_gpio();
	printf("Start screen\n");
	while(screens.exit==false){
		if(curr_screen.change)
			change_screens()
		if(curr_screen.id_current_screen == FRONTPAGE){
			start_screen();
			while(curr_screen.change==false);
		}
		else if(curr_screen.id_current_screen == GAMEPAGE)
			start_game();
	}

	//Unmap and close file
	close(gpio);
	munmap(fbp, framebuffer_size);
	close(fbfd);
	exit(EXIT_SUCCESS);
	return 0;
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
	// printf("Hello from sigio_handler\n");
	uint32_t gamepad_status;
	read(gpio,&gamepad_status,1);
	int i = 0;
	while(!(gamepad_status>>i & 1))
		i++;
	switch(i){
		case 1:
		case 5:
	      	dir = DOWN;
	      break;
	    case 3:
	    case 7:
	      	dir = UP;
	      break;
	    case 2:
	    case 6:
	      	dir = RIGHT;
	      break;
	    case 0:
	      	dir = LEFT;
	    case 4:
	    	dir = ENTER;
	      break;
	}
	// printf(curr_screen.id_current_screen);
	if(curr_screen.id_current_screen == FRONTPAGE)
	{
		printf("frontscreen is active\n");
		for(int i = 0;i<frontscreen.items;i++){
			frontscreen.links[i].status = NOT_SELECTED;
		}

		if(dir == UP){
			if(++frontscreen.position>frontscreen.items - 1)
				frontscreen.position = 0;
		}
		else if(dir == DOWN){
			if(--frontscreen.position < 0)
				frontscreen.position = frontscreen.items - 1;
		}
		frontscreen.links[frontscreen.position].status = SELECTED;
		
		if(dir == UP || dir == DOWN)
		{
			for(int i = 0;i<frontscreen.items;i++){
				selected_background(frontscreen.links[i].x,frontscreen.links[i].y,frontscreen.links[i].length,frontscreen.links[i].status);
			}
		}else if(dir == ENTER){
			if(frontscreen.position == 0)
				curr_screen.change = true;
		}
	}else if(curr_screen.id_current_screen == GAMEPAGE)
	{
		square_box.velocity = 5;
	}
	// printf("gp status: %x \n",(unsigned int)gamepad_status&0xFF);
	return;
}

void change_screens(){
	if(curr_screen.id_current_screen == FRONTPAGE){
		curr_screen.id_current_screen = GAMEPAGE;
	}else if(curr_screen.id_current_screen == GAMEPAGE){
		curr_screen.id_current_screen == FRONTPAGE;	
	}
	curr_screen.change = false;
}


void start_screen(){
	display_string(frontscreen.links[0].x,frontscreen.links[0].y,frontscreen.links[0].string,frontscreen.links[0].length, WHITE);
	display_string(frontscreen.links[1].x,frontscreen.links[1].y,frontscreen.links[1].string,frontscreen.links[1].length, WHITE);
	display_string(frontscreen.links[2].x,frontscreen.links[2].y,frontscreen.links[2].string,frontscreen.links[2].length, WHITE);
	selected_background(frontscreen.links[0].x,frontscreen.links[0].y,frontscreen.links[0].length,frontscreen.links[0].status);
	selected_background(frontscreen.links[1].x,frontscreen.links[1].y,frontscreen.links[1].length,frontscreen.links[1].status);
	selected_background(frontscreen.links[2].x,frontscreen.links[2].y,frontscreen.links[2].length,frontscreen.links[2].status);
	// strcpy(curr_screen.current_screen,frontscreen.screen_name);
	frontscreen.position = 0;
	curr_screen.id_current_screen = frontscreen.id;
	curr_screen.exit = false;
	// frontscreen.position = 0;
}


void start_game(){

	//Init functions
	printf("Start game\n");
	curr_screen.id_current_screen = GAMEPAGE;
	// remove everything on the screen
	int background_color = BLACK;
	printf("updating screen\n");
	for(int i = 0; i < framebuffer_size; i++)
		fbp[i] = background_color;
	update_screen(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	printf("Screen updated\n");
	//initialize main character
	square_box.x = SCREEN_WIDTH/2-10/2;
	square_box.y = SCREEN_HEIGHT/2-10/2;
	square_box.last_y = square_box.y;
	square_box.width = 10;
	square_box.height = 10;
	square_box.velocity = -5;
	printf("initialized box struct\n");

	int acceleration = -1;
	int refrash_rate_us = 500000;
	int refrash_rate_s = 1;//refrash_rate_us/1000000;
	int new_posy;
	printf("Drawing player\n");
	draw_item(square_box.x, square_box.y, square_box.width, square_box.height, NULL);

	printf("Start loop\n");
	// start game loop
	while(1)
	{
		usleep(refrash_rate_us);// pause for x microseconds
		//update position
		square_box.last_y = square_box.y;
		new_posy = square_box.y - square_box.velocity*refrash_rate_s;
		printf("last posy %i\nnewposy %i\n", square_box.y, new_posy);
		if(new_posy+square_box.height <= SCREEN_HEIGHT && new_posy > 0)
			square_box.y =  new_posy;
		//update velocity
		if(square_box.velocity > -5)
			square_box.velocity = square_box.velocity + acceleration*(refrash_rate_s);
		// check collisions
		printf("velocity %i\n", square_box.velocity);
		//update screen by erasing last position and drawing new
		erase_item(square_box.x, square_box.last_y, square_box.width, square_box.height, background_color);
		draw_item(square_box.x, square_box.y, square_box.width, square_box.height, NULL);
	}
}



void selected_background(int x,int y,int width, int status){
	int height = 8;
	int start_xy = x+(y-1)*SCREEN_HEIGHT;
	int stop_xy = (x+width) + (y+height+2)*SCREEN_WIDTH;
	int font_color = WHITE;
	int background = BLACK;
	int selected_background;
	if(status == 1){
		selected_background = GRAY;
	}else{
		selected_background = background;
	}
	for(int i=start_xy;i< stop_xy;i++){
		if(fbp[i]!=font_color){
			fbp[i] = selected_background;
		}
	}
	update_screen(x, y, width, height);
}

