#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

/*Function declarations*/
void display_char(int, int, unsigned char, int);
void display_string(int, int, char [], int,int);
void update_screen(int, int, int, int);

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

extern uint16_t* fbp;

struct Game_frontpage{
	int id;
	int items;
	struct menulink{
		int x;
		int y;
		char string[50];
		int length;
		int status;
		int order;
	} links[3];
};
struct Game_screen{
	int id_current_page;
	int position;
};
struct Game_play{
	int id;
	int player_score;
	char player_score_string[3];
	int num_of_pillars;
	struct pillar{
		int x_position; //leftmost edge of the pillar(towards the player)
		int y_gap_center; //gap between the top and the bottom of the opening between pillars
		int gave_score;
	} pillars[3];
};
struct Player{
	int position;
	int velocity;
	int boost;
	int tick_since_action;
};
struct Game_highscore{
	int id;
	struct highscores{
		char player_score_string[3];
	} highscore[5];
};
struct game_over{
	int id;
};
struct Game_exit{
	int pressed;
};
