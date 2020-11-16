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




