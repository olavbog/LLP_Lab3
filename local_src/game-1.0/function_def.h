#include <stdint.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <signal.h> 
#include <unistd.h> 
#include <stdlib.h> 

/*Function declarations*/
void display_char(int x, int y, unsigned char ch, int color);
void display_string(int x, int y, char string[], int color);
void update_screen(int x, int y, int width, int height);

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

extern uint16_t* fbp;

struct menulink{
	int x;
	int y;
	char[];
	int length;
};

struct Game_frontpage{ 
	char[50];
	struct menulink{
		int x;
		int y;
		char[50];
		int length;
	} new_game, highscores, exit_game;
};
struct Game_frontpage frontpage = {
	"FrontPage",
	{SCREEN_WIDTH/2 - 8/2, SCREEN_HEIGHT/3+00, "New Game",8},
	{SCREEN_WIDTH/2 - 10/2, SCREEN_HEIGHT/3+10, "Highscores",10},
	{SCREEN_WIDTH/2 - 10/2, SCREEN_HEIGHT/3+20, "Exit Game",9},
};

