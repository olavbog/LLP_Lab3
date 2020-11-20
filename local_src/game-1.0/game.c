#define _GNU_SOURCE

#include <stdio.h> //printf
#include <linux/fb.h> //framebuffer
//declares uintx_t
#include <sys/mman.h> // adds mmap and ioctl functions
#include <unistd.h>  //adds close() and usleep()
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "function_def.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SELECTED     1
#define NOT_SELECTED 0

#define FRONTPAGE 0
#define PLAYPAGE 1
#define SCOREPAGE 2
#define GAMEOVERPAGE 3
#define GAME_TICK_TIME 200000

// Defines for player
#define PLAYER_GRAVITY 1
#define STARTPOSITION 80 // height the bird will spawn at
#define BIRDSIZE 16

// Defines for pillars
#define PILLAR_WIDTH 30
#define DISTANCE_BETWEEN_PILLARS 117 //lowest distance for 3 pillars
#define PILLAR_GAP 50
#define PILLAR_SPEED 2

/*Framebuffer variables*/
int fbfd; 						//File open
struct fb_copyarea rect; 		//Defines the area that is updated in the framebuffer
struct fb_var_screeninfo vinfo; //Can receive info about the screen - resolution
uint16_t* fbp; 					//Memory mapping - 16 bits per pixel on the screen

enum direction{UP,DOWN,RIGHT,LEFT,ACTION} dir;


/*GPIO variables*/
int gpio; //File open

struct Game_frontpage page_front = {
	 FRONTPAGE,
	 3, //number of links on the frontpage
	{
		{SCREEN_WIDTH/2 - 8/2*8, SCREEN_HEIGHT/3+00, "New Game",8*8,SELECTED, 0},
		{SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3+10, "Highscores",10*8, NOT_SELECTED,1},
		{SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3+20, "Exit Game",9*8, NOT_SELECTED,2},
	}
};
struct Game_screen game_screen;

struct Game_play game_play = {
		PLAYPAGE, //ID
		STARTPOSITION, // Player start-hight on screen
		0, // player's score
		3, // number of pillars
	 {
	//{x_position, y_gap_center, gave_score}
		{x_position, y_gap_center, NOT_SELECTED}, // pillar 1
		{x_position, y_gap_center, NOT_SELECTED}, // pillar 2
		{x_position, y_gap_center, NOT_SELECTED}, // pillar 3
	 }
};
struct Player player = {
			STARTPOSITION,
			0, // player's velocity
			10, // player's boost
			0, // game ticks since last button push
};
struct Game_highscore game_highscore = {
		SCOREPAGE,
		{
			{"000"}, {"000"}, {"000"}, {"000"}, {"000"},
		}
};
struct Game_over game_over = { GAMEOVERPAGE, };
struct Game_exit game_exit = { 0, }


/*Function declarations*/

int init_gpio();
void sigio_handler(int);
void start_screen();
void selected_background(int,int,int,int);
void update_screen(int, int, int, int);
void spawn_map();
void update_bird();
void update_velocity();
void remove_bird(int);
void display_score();
void collision();
void init_pillar();
void spawn_pillar();
void update_pillar();
void remove_pillar(int);
void draw_pillar(int);


/*----------------------------------------------------------------------*/
/*                                 MAIN                                 */
/*----------------------------------------------------------------------*/

int main(int argc, char *argv[]){

	fbfd = open("/dev/fb0",O_RDWR); //Open framebuffer driver
	ioctl(fbfd,FBIOGET_VSCREENINFO, &vinfo); //Get information about screen

	int framebuffer_size = vinfo.xres*vinfo.yres;//vinfo.smem_len; //Size of the framebuffer
	fbp = mmap(NULL, framebuffer_size, PROT_READ | PROT_WRITE,MAP_SHARED, fbfd, 0);

	//Set white background
	for(int i = 0; i < framebuffer_size; i++)
		fbp[i] = BLACK;
	update_screen(0,0,vinfo.xres,vinfo.yres);
	display_string(SCREEN_WIDTH/2 -(13*8)/2, 1, "Flappy dog",13, WHITE);

	init_gpio();
	printf("Start screen\n");
	start_screen();
	srand(time(NULL)); // Init of random gen for pillar generate
	while(1){
		if(game_screen.id_current_page == game_play.id){ // is the game running?
			//  - add game-tick timer(https://developer.ibm.com/tutorials/l-timers-list/)?
			usleep(GAME_TICK_TIME);
			update_pillar();
			update_bird();
			if (collision() != 0){ // check for collision. if not, update score
				//save_score();
				game_screen.id_current_page = game_over.id;
			}
			display_score();
			// wait for game-tick timer wakeup, can use delay until timer is implemented
		} else if(game_screen.id_current_page == game_over.id) { // game over, freeze screen
			// Write "Game Over" in the middle of the screen
			display_string(SCREEN_WIDTH/2 - 9/2*8, SCREEN_HEIGHT/3+10, "Game Over", 9*8, WHITE);
			// wait for button input. If pushed go back to main menu
			while(game_screen.id_current_page == game_over.id);
			printf("Start screen\n");
			start_screen();
		}
		if(Game_exit.pressed){ // Exit has been pressed on main menu
			break; // Break ininite while loop
		};
	};

	//Unmap and close file
	close(gpio);
	munmap(fbp, framebuffer_size);
	close(fbfd);
	exit(EXIT_SUCCESS);
	return 0;
}

//void save_score()
//{
//	for(int i = 0;i<5;i++){


	}
}

void display_score()
{
	display_string(SCREEN_WIDTH-40, 10, Game_play.player_score_string , 3*8, WHITE);
}

int collision()
{
	if(player.position < BIRDSIZE/2 || player.position > SCREEN_HEIGHT-BIRDSIZE/2){ // Top and bottom of screen
		return -1;
	}
	for(int i = 0;i<game_play.num_of_pillars;i++){
		if (game_play.pillar[i].x_position < (SCREEN_WIDTH/4)+BIRDSIZE/2 && game_play.pillar[i].x_position > (SCREEN_WIDTH/4)-(BIRDSIZE/2)-PILLAR_WIDTH) {
			if(player.position < game_play.pillar[i].y_gap_center - PILLAR_GAP/2 || player.position > game_play.pillar[i].y_gap_center + PILLAR_GAP/2){
				return -1;
			}
		}
		// check if score should be updated
		if ((game_play.pillar[i].x_position) < (SCREEN_WIDTH/4)-(BIRDSIZE/2)-PILLAR_WIDTH){
			if(game_play.pillar[i].gave_score == 0){
				game_play.pillar[i].gave_score = 1;
				game_play.player_score += 1;
				game_play.player_score_string[0] = game_play.player_score % 10;
				game_play.player_score_string[1] = (game_play.player_score % 100)/10;
				game_play.player_score_string[2] = game_play.player_score/100;
				printf("Score updated\n");
			}
		}
	}
	return 0; // collision did not occur.
}

void init_pillar()
{
	spawn_pillar(0, PILLAR_WIDTH+SCREEN_WIDTH);
	spawn_pillar(1, PILLAR_WIDTH+SCREEN_WIDTH+DISTANCE_BETWEEN_PILLARS);
	spawn_pillar(2, PILLAR_WIDTH+SCREEN_WIDTH+DISTANCE_BETWEEN_PILLARS*2);
}

void spawn_pillar(int pillarnr, int x_position)
{
	game_play.pillars[pillarnr].x_position = x_position;
	game_play.pillars[pillarnr].y_gap_center = rand() % (SCREEN_HEIGHT-2*PILLAR_GAP) + PILLAR_GAP;
	game_play.pillar[pillarnr].gave_score = 0;
}

void update_pillar()
{
	for(int i = 0;i<game_play.num_of_pillars;i++){
		if (game_play.pillars[i].x_position == -PILLAR_WIDTH){
			spawn_pillar(i, num_of_pillars*DISTANCE_BETWEEN_PILLARS-PILLAR_WIDTH);
	  }else {
			game_play.pillars[i].x_position -= PILLAR_SPEED;
		}
		if(game_play.pillars[i].x_position < SCREEN_WIDTH){
			remove_pillar(i);
			draw_pillar(i);
		}
	}
}

void remove_pillar(int i){
	if(game_play.pillars[i].x_position < SCREEN_WIDTH-PILLAR_WIDTH){
		draw_rect(game_play.pillars[i].x_position+PILLAR_WIDTH, 0, PILLAR_SPEED, 2*SCREEN_HEIGHT/3, BLUE);
		draw_rect(game_play.pillars[i].x_position+PILLAR_WIDTH, 2*SCREEN_HEIGHT/3, PILLAR_SPEED, SCREEN_HEIGHT/3, GREEN);
	}
}

void draw_pillar(int i)
{
	if(game_play.pillars[i].x_position >= 0){ //dont draw outside left edge
		draw_rect(game_play.pillars[i].x_position, 0, PILLAR_SPEED, game_play.pillars[i].y_gap_center-PILLAR_GAP/2, YELLOW);
		draw_rect(game_play.pillars[i].x_position, game_play.pillars[i].y_gap_center+PILLAR_GAP/2 , PILLAR_SPEED, SCREEN_HEIGHT-game_play.pillars[i].y_gap_center+PILLAR_GAP/2, YELLOW);
	}
}

void update_screen(int x, int y, int width, int height)
{
	rect.dx = x;
	rect.dy = y;
	rect.width  = width;
	rect.height = height;
	ioctl(fbfd, 0x4680, &rect);
}

int init_gpio()
{
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

void sigio_handler(int no)
{
	// printf("Hello from sigio_handler\n");
	uint32_t gamepad_status;
	read(gpio,&gamepad_status,1);
	int i = 0;
	while(!(gamepad_status>>i & 1))
		i++;
	switch(i){
		case 1:
	      dir = ACTION;
		    break;
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
	    case 4:
	      dir = LEFT;
	      break;
	}

	// printf(game_screen.id_current_page);
	if(game_screen.id_current_page == page_front.id){
		if(dir == ACTION){
			switch(game_screen.position){
					case 0: // new game
						spawn_map();
						game_play.player_score = 0;
						game_play.player_score_string = "000";
						game_screen.id_current_page = game_play.id;
						break;
					case 1: // high score
						game_highscore.id_current_page = 2;
						break;
					case 2: // exit game
						Game_exit.pressed = SELECTED;
						break;
			}
		}
		else{
			printf("frontpage is active\n");
			for(int i = 0;i<page_front.items;i++){
				page_front.links[i].status = NOT_SELECTED;
			}

			if(dir == UP){
				game_screen.position++;
			}else if(dir == DOWN){
				game_screen.position--;
			}
			if(game_screen.position < 0){
				game_screen.position = page_front.items - 1;
			}else if(game_screen.position > page_front.items - 1){
				game_screen.position = 0;
			}
			page_front.links[game_screen.position].status = SELECTED;

			if(dir == UP || dir == DOWN){
				for(int i = 0;i<page_front.items;i++){
					selected_background(page_front.links[i].x,page_front.links[i].y,page_front.links[i].length,page_front.links[i].status);
				}
			}
		}
	} else if(game_screen.id_current_page == game_play.id){ //game is running
		if(dir == ACTION){
			if(player.velocity < 0){
				player.tick_since_action = 0;
			}

		}
	} else if(game_screen.id_current_page == game_over.id){
		game_screen.id_current_page =  page_front.id;
	}

	// printf("gp status: %x \n",(unsigned int)gamepad_status&0xFF);
	return;
}

void start_screen()
{
	display_string(page_front.links[0].x,page_front.links[0].y,page_front.links[0].string,page_front.links[0].length, WHITE);
	display_string(page_front.links[1].x,page_front.links[1].y,page_front.links[1].string,page_front.links[1].length, WHITE);
	display_string(page_front.links[2].x,page_front.links[2].y,page_front.links[2].string,page_front.links[2].length, WHITE);
	selected_background(page_front.links[0].x,page_front.links[0].y,page_front.links[0].length,page_front.links[0].status);
	selected_background(page_front.links[1].x,page_front.links[1].y,page_front.links[1].length,page_front.links[1].status);
	selected_background(page_front.links[2].x,page_front.links[2].y,page_front.links[2].length,page_front.links[2].status);
	// strcpy(game_screen.current_page,page_front.page_name);
	game_screen.id_current_page = page_front.id;
	game_screen.position = 0;
}

void spawn_map()
{ // Set initial background in preperation for game start.
	for(int i = 0; i < (2*framebuffer_size/3)-1; i++)
		fbp[i] = BLUE; // Blue for sky
	for(int i = 2*framebuffer_size/3; i < framebuffer_size; i++)
		fbp[i] = GREEN; // Green for grass
	draw_bird(player.position);

	update_screen(0,0,vinfo.xres,vinfo.yres);
}

void update_bird()
{
	if(player.velocity != 0){
		remove_bird(player.position);
		//update y_position
		player.position += player.velocity;
		draw_bird(player.position);
	}
}

void update_velocity()
{
	player.velocity = player.boost - PLAYER_GRAVITY * player.tick_since_action;
	player.tick_since_action += 1;
}

void draw_bird(int position)
{
	draw_rect(SCREEN_WIDTH/4, position-BIRDSIZE/2, BIRDSIZE, BIRDSIZE, BLACK;
}


void remove_bird(int position)
{
	int birdsize_in_blue = 0; // If the first if-sentence fails the whole background should be green
	//need to remove old bird
	if((position-BIRDSIZE/2) < 2*SCREEN_HEIGHT/3){
		draw_rect(SCREEN_WIDTH/4, position-BIRDSIZE/2, BIRDSIZE, BIRDSIZE, BLUE);

		if (2*SCREEN_HEIGHT/3 - position-BIRDSIZE/2) < 16){ // check if some of the box is over the line between blue and green
			birdsize_in_blue = 2*SCREEN_HEIGHT/3 - position-BIRDSIZE/2;
		}
	}
	if((position+BIRDSIZE/2) > 2*SCREEN_HEIGHT/3){
		draw_rect(SCREEN_WIDTH/4, position+BIRDSIZE/2, BIRDSIZE, BIRDSIZE-birdsize_in_blue, GREEN);
	}
}


void selected_background(int x,int y,int width, int status)
{
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
