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