// Some drawing function i made for the fun of it
void draw_rect(int x, int y, int width, int height){

	for(int row = y; row < height+y; row++)
	{
		for(int column = x; column < x+width; column++)
		{
			fbp[column + row*320]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
	printf("Drew rect");
}

void draw_triangle(int x,int y, int height){
	int width = 320;
	fbp[x + y*width]=0xFFFF;
	fbp[x + (y+height*2)*width]=0xFFFF;
	for(int row = 0; row < 2*height; row++)
	{
		for(int pos = -row-1; pos<row+1;pos++)
		{
			fbp[x+pos+(y+row)*width]=0xFFFF;
			fbp[x+pos+(y+4*height-row-2)*width]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
}


void draw_pixel(int x,int y)
{
	int width = 320;
	for(int j = 0; j<10;j++)
	{
		for(int i = x; i<x+10; i++)
		{
			fbp[i+(y+j)*width]=0xFFFF;
		}
	}
	ioctl(fbfd, 0x4680, &rect);
}