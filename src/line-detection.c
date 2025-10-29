#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/image_rotation.h"
#include "../include/image_treatment.h"
#include "../include/image_slice.h"
#include "../include/image_helpers.h"



void sobel_filter(GdkPixbuf *pixbuf)
{
	int Gx[3][3] = {{-1, 0, 1},
			{-2, 0, 2},
			{-1, 0, 1}};
    	int Gy[3][3] = {{-1, -2, -1},
			{0, 0, 0},
			{1, 2, 1}};
	int gx = 0;
	int gy = 0;

	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	
	int *temp_p = malloc(width * height * sizeof(int));
	for(int i=0;i<width*height;i++)
		temp_p[i]=0;

	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

	for(int i = 1; i < height - 1; i ++)
	{
		for(int j = 1; j < width - 1; j ++)
		{
			gx = 0;
			gy = 0;	
			for(int sob_y = 0; sob_y <= 2; sob_y ++)
			{
				for(int sob_x = 0; sob_x <= 2; sob_x ++)
				{
					guchar *p = pixels + (i + sob_y - 1)*rowstride + (j + sob_x - 1)*n_channels;
					gx += Gx[sob_y][sob_x]* (*p);
					gy += Gy[sob_y][sob_x]* (*p);
				}
			}
			int diff = (int)sqrt(gx*gx + gy*gy);
                	if(diff > 255)
                		diff = 255;
                	temp_p[i * width + j] = diff;
		}
	}

	for(int i = 0; i < height; i ++)
	{
		for(int j = 0; j < width; j ++)
		{
			guchar *p = pixels + i*rowstride + j*n_channels;
			p[0] = p[1] = p[2] = (guchar)temp_p[i * width + j];
			if(n_channels==4)
				p[3] = 255;
		}
	}

	free(temp_p);
}





void pipeline(char *filename)
{
	GdkPixbuf *pixbuf = load_image(filename);
	convert_to_grayscale(pixbuf);
	sobel_filter(pixbuf);
	binarize_image(pixbuf, 180);
	save_pixbuf_as_png(pixbuf, filename);
}

void main(int argc, char *argv[])
{
	pipeline(argv[1]);
}

