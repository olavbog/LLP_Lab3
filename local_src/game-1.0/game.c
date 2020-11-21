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
#include <time.h>

#include <stdbool.h> // includes bool

#include "colors.h"
#include "function_def.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define SELECTED     1
#define NOT_SELECTED 0

#define FRONTSCREEN 0
#define GAMESCREEN 1
#define SCORESCREEN 2
#define GAMEOVERSCREEN 3

#define GAME_TICK_TIME 100000

// Defines for player
#define PLAYER_GRAVITY 1
#define STARTPOSITION 80 // height the bird will spawn at
#define BIRDSIZE 16
#define MAX_PLAYER_VELOCITY 4

// Defines for pillars
#define PILLAR_WIDTH 30
#define DISTANCE_BETWEEN_PILLARS 117 //lowest distance for 3 pillars
#define PILLAR_GAP 50
#define PILLAR_SPEED 2

/*Framebuffer variables*/
int fbfd; 						//File open
struct fb_var_screeninfo vinfo; //Can receive info about the screen - resolution
uint16_t* fbp; 					//Memory mapping - 16 bits per pixel on the screen

// struct Game_character square_box;
int framebuffer_size;


enum direction{UP,DOWN,RIGHT,LEFT,ACTION} dir;



/*GPIO variables*/
int gpio; //File open

struct Game_frontscreen frontscreen = {
	 FRONTSCREEN,
	 3, //number of links on the frontscreen
	 0,
	{
		{SCREEN_WIDTH/2 - 8/2*8 , SCREEN_HEIGHT/3+00, "New Game",   8*8, SELECTED,    0},
		{SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3+10, "Highscores",10*8, NOT_SELECTED,1},
		{SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3+20, "Exit Game",  9*8, NOT_SELECTED,2},
	}
};
struct screens curr_screen;

struct Game_play game_play = {
		GAMESCREEN, //ID
		0, // player's score
		"000",
		3, // number of pillars
	 {
	//{x_position, y_gap_center, gave_score}
		{0, 0, NOT_SELECTED}, // pillar 1 //x pos is at leftmost edge
		{0, 0, NOT_SELECTED}, // pillar 2
		{0, 0, NOT_SELECTED}, // pillar 3
	 }
};
struct Player player = {
			STARTPOSITION, //y pos is at center of bird
			0, // player's velocity
			10, // player's boost
			0, // game ticks since last button push
};
struct Game_highscore game_highscore = {
	SCORESCREEN,
 	{
		{0,"000"}, {0,"000"}, {0,"000"}, {0,"000"}, {0,"000"},
	}
};
struct Game_over game_over = { GAMEOVERSCREEN };
// struct Game_exit game_exit = { 0, };


/*Function declarations*/

int init_gpio();
void sigio_handler(int);
void start_screen();
void selected_background(int,int,int,int);


// void change_screens();
void spawn_map();
void draw_bird(int);
void update_bird();
void update_velocity();
void remove_bird(int);
void display_score();
void score_screen();
void save_score();
int collision();
void init_pillar();
void spawn_pillar();
void update_pillar();
void remove_pillar(int);
void draw_pillar(int);

// use draw item instead
// void draw_rect(int, int, int, int, uint16_t);


/*----------------------------------------------------------------------*/
/*                                 MAIN                                 */
/*----------------------------------------------------------------------*/

int main(int argc, char *argv[]){

	fbfd = open("/dev/fb0",O_RDWR); //Open framebuffer driver
	ioctl(fbfd,FBIOGET_VSCREENINFO, &vinfo); //Get information about screen

	framebuffer_size = vinfo.xres*vinfo.yres;//vinfo.smem_len; //Size of the framebuffer
	fbp = mmap(NULL, framebuffer_size, PROT_READ | PROT_WRITE,MAP_SHARED, fbfd, 0);

	//Set white background
	for(int i = 0; i < framebuffer_size; i++)
		fbp[i] = BLACK;
	update_screen(0,0,vinfo.xres,vinfo.yres);
	display_string(SCREEN_WIDTH/2 -(13*8)/2, 1, "Flappy dog",13, WHITE);

	init_gpio();
	printf("Start screen\n");

	srand(time(0)); // Init of random gen for pillar generate
	start_screen();
	while(curr_screen.exit==false){
		if(curr_screen.id_current_screen == FRONTSCREEN){
			usleep(10);
		}else if(curr_screen.id_current_screen == GAMESCREEN){ // is the game running?
			//  - add game-tick timer(https://developer.ibm.com/tutorials/l-timers-list/)?
			// printf("current screen is GAMESCREEN\n");
			usleep(GAME_TICK_TIME);
			update_pillar();
			update_bird();
			if (collision() != 0)
			{ // check for collision. if not, update score
				//save_score();
				printf("Collision detected\n");
				curr_screen.id_current_screen = GAMEOVERSCREEN;
			}
			display_score();
			printf("Loop done\n");
			// wait for game-tick timer wakeup, can use delay until timer is implemented
		} else if(curr_screen.id_current_screen == GAMEOVERSCREEN) { 	// game over, freeze screen
			// Write "Game Over" in the middle of the screen
			printf("current screen is GAMEOVERSCREEN\n");
			
			display_string(SCREEN_WIDTH/2 - 9/2*8, SCREEN_HEIGHT/3+10, "Game Over", 9, WHITE);
			display_string(SCREEN_WIDTH/2 - 27*8/2, SCREEN_HEIGHT/3+20, "Push any button to continue", 27, WHITE);
			// wait for button input. If pushed go back to main menu
			usleep(10);
			printf("Wrote to screen \n");
			player.position = STARTPOSITION;

			while(curr_screen.id_current_screen == GAMEOVERSCREEN)
				usleep(10); //small delayed needed for the variable to be able to change
			start_screen();
		} else if(curr_screen.id_current_screen == SCORESCREEN) {
			score_screen();
			while(curr_screen.id_current_screen == SCORESCREEN)
				usleep(10); //small delayed needed for the variable to be able to change
			start_screen();
		}
		// if(game_exit.pressed){ // Exit has been pressed on main menu
		// 	break; // Break ininite while loop
		// };
	};
	printf("Exiting");

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


//	}
//}

void score_screen()
{
	draw_item(0,0,vinfo.xres,vinfo.yres,BLACK,NULL); // Clean screen
	display_string(SCREEN_WIDTH/2 - 10/2*8, SCREEN_HEIGHT/3-10,"Highscores",10, WHITE);
	for(int i = 0;i<5;i++){ 
		display_string(SCREEN_WIDTH/2 - 12, SCREEN_HEIGHT/3+10*i,game_highscore.highscore[i].player_score_string,3, WHITE);
	}
	display_string(SCREEN_WIDTH/2 - 26*8/2, SCREEN_HEIGHT/3+60, "Push any button to go back", 26, WHITE);
}

void save_score()
{
	for(int i = 0;i<5;i++){
		if (game_play.player_score > game_highscore.highscore[i].player_score_int){ //check each highscore entry if new score is higher
			for(int j = 0;j<4-i;j++){ // If new highscore is found, move [3] to [4], [2] to [3],etc to make space for new entry
				game_highscore.highscore[4-j].player_score_int = game_highscore.highscore[3-j].player_score_int;
				game_highscore.highscore[4-j].player_score_string = game_highscore.highscore[3-j].player_score_string;
			}
			game_highscore.highscore[i].player_score_int = game_play.player_score; //write new highscore entry
			game_highscore.highscore[i].player_score_string = game_play.player_score_string;
			return;
		}
	}
}

void display_score()
{
	display_string(SCREEN_WIDTH-40, 10, game_play.player_score_string , 3*8, WHITE);
}

int collision()
{
	printf("Checking collision \n");
	if(player.position-BIRDSIZE/2 < BIRDSIZE || player.position+BIRDSIZE/2 > SCREEN_HEIGHT-BIRDSIZE/2){ // Top and bottom of screen
		return -1;
	}
	for(int i = 0; i < game_play.num_of_pillars; i++){
		// check if player is within the pillars
		if (game_play.pillars[i].x_position > (SCREEN_WIDTH/4)-PILLAR_WIDTH && game_play.pillars[i].x_position < (SCREEN_WIDTH/4)+BIRDSIZE) {
			//check if player has collided with the pillars
			if(player.position - BIRDSIZE/2 < game_play.pillars[i].y_gap_center - PILLAR_GAP/2){// || player.position+BIRDSIZE/2 > game_play.pillars[i].y_gap_center + PILLAR_GAP/2){
				return -1;
			}
		}
		// check if score should be updated
		if ((game_play.pillars[i].x_position) < (SCREEN_WIDTH/4) - (BIRDSIZE/2)-PILLAR_WIDTH){
			if(game_play.pillars[i].gave_score == 0){
				game_play.pillars[i].gave_score = 1;
				game_play.player_score += 1;
				game_play.player_score_string[0] =  game_play.player_score % 10;
				game_play.player_score_string[1] = (game_play.player_score % 100)/10;
				game_play.player_score_string[2] =  game_play.player_score/100;
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

void spawn_pillar(int pillarnr, int pos_x)
{
	game_play.pillars[pillarnr].x_position   = pos_x;
	game_play.pillars[pillarnr].y_gap_center = rand() % (SCREEN_HEIGHT - 2*PILLAR_GAP) + PILLAR_GAP;
	game_play.pillars[pillarnr].gave_score   = 0;
}

void update_pillar()
{	
	// for(int i = 0; i < game_play.num_of_pillars; i++){
	// 	if (game_play.pillars[i].x_position == -PILLAR_WIDTH){ //can end up in a position where this value is skipped(E.G going from -29 to -31). Should probably be changed to <=
	// 		spawn_pillar(i, game_play.num_of_pillars*DISTANCE_BETWEEN_PILLARS-PILLAR_WIDTH);
	//   	}else{
	// 		game_play.pillars[i].x_position -= PILLAR_SPEED;
	// 	}
	// 	if(game_play.pillars[i].x_position < SCREEN_WIDTH){ // maybe use  <= SCREEN_WIDTH-PILLAR_SPEED. as it now might try to draw out of bounds E.G when position 319 it will try to draw on 320 also because of pillarspeed=2
	// 		remove_pillar(i);
	// 		draw_pillar(i);
	// 	}
	// }
	// spawn_pillar(0, SCREEN_WIDTH);
	remove_pillar(0);
	game_play.pillars[0].x_position -= PILLAR_SPEED;
	draw_pillar(0);
}

void remove_pillar(int i){
	int remove_outside = 0;
	// if(game_play.pillars[i].x_position < SCREEN_WIDTH){
	// if(game_play.pillars[i].x_position+PILLAR_WIDTH/2>SCREEN_WIDTH)
	// 	remove_outside = game_play.pillars[i].x_position+PILLAR_WIDTH - SCREEN_WIDTH;
	// if(remove_outside<PILLAR_WIDTH)
	if(game_play.pillars[i].x_position<SCREEN_WIDTH-PILLAR_WIDTH/2 && game_play.pillars[i].x_position > 0)
		draw_item(game_play.pillars[i].x_position-PILLAR_WIDTH, 0, PILLAR_WIDTH-remove_outside, 2*SCREEN_HEIGHT/3, BLUE, NULL);
	// draw_item(game_play.pillars[i].x_position-PILLAR_WIDTH, 2*SCREEN_HEIGHT/3, PILLAR_WIDTH-remove_outside, SCREEN_HEIGHT/3, GREEN, NULL);
}

void draw_pillar(int i)
{	
	int remove_outside = 0;
	int move_x = 0;
	// if(game_play.pillars[i].x_position < SCREEN_WIDTH){ //dont draw outside left edge 
	// 	if(game_play.pillars[i].x_position<0){
	// 		remove_outside = -game_play.pillars[i].x_position;
	// 		move_x = -game_play.pillars[i].x_position;
	// 	}
	// 	else if(game_play.pillars[i].x_position+PILLAR_WIDTH/2>SCREEN_WIDTH)
	// 		remove_outside = game_play.pillars[i].x_position+PILLAR_WIDTH/2-SCREEN_WIDTH;

	// 	draw_item(game_play.pillars[i].x_position+move_x, 0, PILLAR_WIDTH-remove_outside, game_play.pillars[i].y_gap_center-PILLAR_GAP/2, WHITE, NULL);
	// 	// draw_item(game_play.pillars[i].x_position+remove_outside, game_play.pillars[i].y_gap_center+PILLAR_GAP/2 , PILLAR_WIDTH-remove_outside, SCREEN_HEIGHT-game_play.pillars[i].y_gap_center+PILLAR_GAP/2, WHITE, NULL);
	// }
	if(game_play.pillars[i].x_position<SCREEN_WIDTH - PILLAR_WIDTH/2 && game_play.pillars[i].x_position > 0)
		draw_item(game_play.pillars[i].x_position-PILLAR_WIDTH, 0, PILLAR_WIDTH-remove_outside, game_play.pillars[i].y_gap_center-PILLAR_GAP/2, WHITE, NULL);
}

int init_gpio()
{
	gpio = open("/dev/gamepad", O_RDWR);
	if(gpio < 0){
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
	while(!(gamepad_status>>i & 1) && i<8)
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
	    	dir = ACTION;
	      break;
	    default:
	    	break;
	}
	if(curr_screen.id_current_screen == FRONTSCREEN){
		if(dir == ACTION){
			switch(frontscreen.position){
				case 0: // new game
					spawn_map();
					game_play.player_score = 0;
					//game_play.player_score_string[0] = '0';
					//game_play.player_score_string[1] = '0';
					//game_play.player_score_string[2] = '0';
					memset(&game_play.player_score_string[0], 0, sizeof(game_play.player_score_string));
					curr_screen.id_current_screen = GAMESCREEN;
					break;
				case 1: // high score
					// curr_screen.id_current_screen = SCORESCREEN;
					break;
				case 2: // exit game
					curr_screen.exit = true;
					usleep(10);
					break;
			}
		}
		else{
			for(int i = 0; i < frontscreen.items; i++)
				frontscreen.links[i].status = NOT_SELECTED;

			if(dir == UP){
				if(++frontscreen.position > frontscreen.items - 1)
					frontscreen.position = 0;
			}
			else if(dir == DOWN){
				if(--frontscreen.position < 0)
					frontscreen.position = frontscreen.items - 1;
			}
			frontscreen.links[frontscreen.position].status = SELECTED;
		
			for(int i = 0;i<frontscreen.items;i++){
				selected_background(frontscreen.links[i].x,frontscreen.links[i].y,frontscreen.links[i].length,frontscreen.links[i].status);
			}
		}
	} else if(curr_screen.id_current_screen == GAMESCREEN){ //game is running
		if(dir == ACTION){
			// if(player.velocity < 0){ // The idea here was to prevent the player from spamming the jump button
			// 	player.tick_since_action = 0;
			// }
			player.velocity = 7;
		}
	} else if(curr_screen.id_current_screen == GAMEOVERSCREEN){
		curr_screen.id_current_screen = FRONTSCREEN;
	} else if(curr_screen.id_current_screen == SCORESCREEN){
		curr_screen.id_current_screen = FRONTSCREEN;
	}
	// printf("Finished sigio\n");
	usleep(10);
	return;
}

void start_screen()
{
	draw_item(0,0,vinfo.xres,vinfo.yres,BLACK,NULL);
	display_string(frontscreen.links[0].x,frontscreen.links[0].y,frontscreen.links[0].string,frontscreen.links[0].length, WHITE);
	display_string(frontscreen.links[1].x,frontscreen.links[1].y,frontscreen.links[1].string,frontscreen.links[1].length, WHITE);
	display_string(frontscreen.links[2].x,frontscreen.links[2].y,frontscreen.links[2].string,frontscreen.links[2].length, WHITE);
	selected_background(frontscreen.links[0].x,frontscreen.links[0].y,frontscreen.links[0].length,frontscreen.links[0].status);
	selected_background(frontscreen.links[1].x,frontscreen.links[1].y,frontscreen.links[1].length,frontscreen.links[1].status);
	selected_background(frontscreen.links[2].x,frontscreen.links[2].y,frontscreen.links[2].length,frontscreen.links[2].status);

	curr_screen.id_current_screen = FRONTSCREEN;
	curr_screen.exit = false;
	frontscreen.position = 0;
}

void spawn_map()
{ // Set initial background in preperation for game start.
	printf("Spawn map\n");	
	for(int i = 0; i < vinfo.xres*vinfo.yres; i++)
		if(i<2*vinfo.xres*vinfo.yres/3)
			fbp[i] = BLUE; // Blue for sky
		else
			fbp[i] = GREEN; // Green for grass
	init_pillar();
	draw_bird(player.position);
	update_screen(0,0,vinfo.xres,vinfo.yres);
}

void update_bird()
{
	printf("update bird\nvelocity %i\n",player.velocity);
	remove_bird(player.position);

	//update y_position
	player.position -= player.velocity;
	draw_bird(player.position);
	//accellerate
	player.velocity -= PLAYER_GRAVITY;
}

// void update_velocity()
// {
// 	player.velocity = player.boost - PLAYER_GRAVITY * player.tick_since_action;
// 	player.tick_since_action += 1;
// }

void draw_bird(int position)
{	
	draw_item(SCREEN_WIDTH/4, position-BIRDSIZE/2, BIRDSIZE, BIRDSIZE, BLACK, NULL);
}


void remove_bird(int position)
{
	int birdsize_in_blue = 0; // If the first if-sentence fails the whole background should be green
	//need to remove old bird
	printf("Y position %i\n",position);
	if((position-BIRDSIZE/2) <= 2*SCREEN_HEIGHT/3){
		draw_item(SCREEN_WIDTH/4, position-BIRDSIZE/2, BIRDSIZE, BIRDSIZE, BLUE, NULL);

		// if ((2*SCREEN_HEIGHT/3 - position-BIRDSIZE/2) < 16){ // check if some of the box is over the line between blue and green
		// 	birdsize_in_blue = 2*SCREEN_HEIGHT/3 - position-BIRDSIZE/2;
		// }
	}else{
		draw_item(SCREEN_WIDTH/4, position-BIRDSIZE/2, BIRDSIZE, BIRDSIZE, GREEN, NULL);
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
	for(int i=start_xy; i< stop_xy; i++){
		if(fbp[i]!=font_color){
			fbp[i] = selected_background;
		}
	}
	update_screen(x, y, width, height);
}

// void draw_rect(int x, int y, int width, int height, uint16_t color){

// 	for(int row = y; row < height+y; row++)
// 	{
// 		for(int column = x; column < x+width; column++)
// 		{
// 			fbp[column + row*320]=color;
// 		}
// 	}
// 	ioctl(fbfd, 0x4680, &rect);
// 	printf("Drew rect");
// }
