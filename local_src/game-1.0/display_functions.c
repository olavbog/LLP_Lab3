#include <stdio.h> //printf
#include <sys/mman.h>
#include <sys/ioctl.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <linux/fb.h> //framebuffer


#include "font8x8.h"
#include "function_def.h"
#include "colors.h"

struct fb_copyarea rect; 		//Defines the area that is updated in the framebuffer

void update_screen(int x, int y, int width, int height){
	rect.dx = x;
	rect.dy = y;
	rect.width  = width;
	rect.height = height;
	ioctl(fbfd, 0x4680, &rect);
}


void draw_item(int x, int y, int width, int height, uint8_t* pixel_map)
{
	printf("draw_item()\n");
	// int color = GREEN;
	uint16_t color = 0x001F;
	if(pixel_map == NULL)
	{
		printf("Drawing box\n");
		for(int row_y=0; row_y < height; row_y++){
			for(int col_x=0; col_x < width; col_x++)
				fbp[(x+col_x)+(y+row_y)*SCREEN_WIDTH] = 0xFFFF;
		}
	}
	else{
		printf("You're on the wrong side of town cowboy\n");
		//insert pixel map function here
		// very similar to previous, but will draw
		// what is sent through the pixel map
		// pointer
	}
	update_screen(x, y, width, height);
}
void erase_item(int x, int y, int width, int height,uint16_t fill_color)
{
	for(int row_y=0; row_y < height; row_y++){
		for(int col_x=0; col_x < width; col_x++)
			fbp[(x+col_x)+(y+row_y)*SCREEN_WIDTH] = fill_color;
	}
	update_screen(x, y, width, height);
}

void display_char(int x, int y, unsigned char ch, uint16_t color)
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

void display_string(int x, int y, char string[], int length, uint16_t color)
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

