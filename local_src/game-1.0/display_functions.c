#include <sys/mman.h>
#include <sys/ioctl.h> 
#include <sys/stat.h> 
#include <sys/types.h>

#include "font8x8.h"
#include "function_def.h"
#include "colors.h"

void display_char(int x, int y, unsigned char ch, int color)
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if(((font8x8_basic[ch][i])>>j) & 1)
			{
				fbp[(x+j)+(i+y)*SCREEN_WIDTH]=color;
			}
		}
	}
}

void display_string(int x, int y, char string[],int length,int color)
{
	/*
		Loops through all chars in the input strings
		and writes it to the LCD screen at the given
		coordinates
	*/
	int i = 0;
	for(;i<length;i++){
		/*
		   x coordinate will increase by 8 for the width of the char
		   and 1 for a space inbetween
		*/
		if(x+(i*8)+8+y*SCREEN_WIDTH > SCREEN_WIDTH*SCREEN_HEIGHT )
		{
			return;
		}
		display_char(x+(i*8),y, string[i],color); 
	}
	update_screen(x,y,8*i,10);
}

