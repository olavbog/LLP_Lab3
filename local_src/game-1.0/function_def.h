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
void draw_item(int, int, int, int, uint8_t*);
void erase_item(int, int, int ,int ,int);


#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

extern uint16_t* fbp;
extern struct fb_copyarea rect;

struct Game_frontscreen{ 
	int id;
	int items;
	int position;
	struct menulink{
		int x;
		int y;
		char string[50];
		int length;
		int status;
		int order;
	} links[3];
};
struct screens{
	int id_current_page;
};

struct Game_screen{
	int id;
};

struct Game_character{
	int x;
	int y;
	int last_y;
	int width;
	int height;
	int velocity;
};




