#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
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

void invert_color(GdkPixbuf *pixbuf)
{
	int width = gdk_pixbuf_get_width(pixbuf);
    	int height = gdk_pixbuf_get_height(pixbuf);
    	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
        		guchar *p = pixels + y * rowstride + x * n_channels;
        		p[0] = p[1] = p[2] = 255 - p[0];
		}
	}
}

double find_good_rotation(GdkPixbuf *pixbuf)
{
	//need to be dev
	return 0.0;
}

int remove_lines(GdkPixbuf *pixbuf)
{
	//need to be dev
	return 0;
}

static void find_black_pixels_around(GdkPixbuf *pixbuf, int x, int y, int *is_visited, int index_coo, int **coo)
{
	int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	guchar *p = pixels + y * rowstride + x * n_channels;
	
	if(p[0] < 100 && is_visited[y * width + x] == 0)
	{
		int direction[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

		for(int i = 0; i < 8; i ++)
		{
			is_visited[y * width + x] = 1;
			if(x >= coo[index_coo][2] && y >= coo[index_coo][3])
			{
				coo[index_coo][2] = x;
				coo[index_coo][3] = y;
			}
			find_black_pixels_around(pixbuf, x + direction[i][0], y + direction[i][1], is_visited, index_coo, coo);
		}
	}
}

void find_letter(GdkPixbuf *pixbuf, int **coo)
{
	int width = gdk_pixbuf_get_width(pixbuf);
    	int height = gdk_pixbuf_get_height(pixbuf);
    	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

	int *is_visited = malloc(width * height * sizeof(int)); // 0 if False 1 if True
	memset(is_visited, 0, width * height * sizeof(int));
	
	int index_coo = 0;

	for(int y = 0; y < height; y ++)
	{
		for(int x = 0; x < width; x ++)
		{
			if(is_visited[y * width + x] == 0)
			{
				is_visited[y * width + x] = 1;
				guchar *p = pixels + y * rowstride + x * n_channels;
				
				if(p[0] < 100)
				{
					coo[index_coo][0] = coo[index_coo][2] = x;
					coo[index_coo][1] = coo[index_coo][3] = y;
					find_black_pixels_around(pixbuf, x, y, is_visited, index_coo, coo);
					index_coo ++;
				}
			}
		}
	}
	
	free(is_visited);
}

void generate_letter(GdkPixbuf *pixbuf, int **coo, char *output_file)
{
	int index_coo = 0;
	
	char* full_path = malloc(strlen(output_file) + 21 * sizeof(char));
	
	while(coo[index_coo][0] != -1)
	{
		GdkPixbuf letter = crop(pixbuf, coo[index_coo][0], coo[index_coo][1], coo[index_coo][2], coo[index_coo][3]);
		sprintf(full_path, "%s%d_%d", output_file, coo[index_coo][0], coo[index_coo][1]);
		save_pixbuf_as_png(letter, full_path);
		index_coo ++;
		g_object_unref(letter);
	}
	free(full_path);
}

void pipeline(char *filename, char *output_file)
{
	GdkPixbuf *pixbuf = load_image(filename);
	GdkPixbuf *pixbuf_to_slice = load_image(filename);
	int width = gdk_pixbuf_get_width(pixbuf);
    	int height = gdk_pixbuf_get_height(pixbuf);
	int **coo = malloc(width * height * sizeof(int*));
	for(int i = 0; i < width * height)
	{
		coo[i] = malloc(4 * sizeof(int)); // coo[i][0] = x1 coo[i][1] = y1 coo[i][2] = x2 coo[i][3] = y2
		memset(coo[i], -1, 4 * sizeof(int));
	}

	convert_to_grayscale(pixbuf);
	sobel_filter(pixbuf);
	binarize_image(pixbuf, 180);
	invert_color(pixbuf);

	double angle = find_good_rotation(pixbuf);
	pixbuf = rotate_image(pixbuf, angle);
	pixbuf_to_slice = rotate_image(pixbuf_to_slice, angle);
	
	remove_lines(pixbuf);

	find_letter(pixbuf, coo);
	
	generate_letter(pixbuf, coo, output_file);
	
	for(int i = 0; i < width * height; i ++)
	{
		free(coo[i]);
	}
	free(coo);
	g_object_unref(pixbuf);
	g_object_unref(pixbuf_to_slice);
}

void main(int argc, char *argv[])
{
	pipeline(argv[1], argv[2]);
}

