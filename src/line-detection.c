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

void remove_lines(GdkPixbuf *pixbuf)
{
	int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

	for (int y = 0; y < height; y ++)
        {
		int sum = 0;
                for (int x = 0; x < width; x ++)
                {
                        guchar *p = pixels + y * rowstride + x * n_channels;
                        sum += p[0];
                }
		if(sum >= width * 255 * 0.70)
		{
			for(int x = 0; x < width; x ++)
			{
				guchar *p = pixels + y * rowstride + x * n_channels;
				p[0] = p[1] = p[2] = 0;
			}
		}
        }

	for (int x = 0; x < width; x ++)
	{
		 int sum = 0;
                for (int y = 0; y < height; y ++)
                {
                        guchar *p = pixels + y * rowstride + x * n_channels;
                        sum += p[0];
                }
                if(sum >= height * 255 * 0.70)
                {
                        for(int y = 0; y < height; y ++)
                        {
                                guchar *p = pixels + y * rowstride + x * n_channels;
                                p[0] = p[1] = p[2] = 0;
                        }
                }
	}
}

static void find_black_pixels_around(GdkPixbuf *pixbuf, int x, int y, int *is_visited, int index_coo, int **coo)
{
	int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	guchar *p = pixels + y * rowstride + x * n_channels;
	
	if(p[0] == 255 && is_visited[y * width + x] == 0)
	{
		is_visited[y * width + x] = 1;
		int direction[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

		for(int i = 0; i < 8; i ++)
		{
			if (x < coo[index_coo][0])
				coo[index_coo][0] = x;
    			if (y < coo[index_coo][1])
				coo[index_coo][1] = y;
    			if (x > coo[index_coo][2])
				coo[index_coo][2] = x;
    			if (y > coo[index_coo][3])
				coo[index_coo][3] = y;

			if((x + direction[i][0]) >= 0 && (y + direction[i][1]) >= 0 && (x + direction[i][0]) < width &&
					(y + direction[i][1]) < height && (coo[index_coo][2] - coo[index_coo][0] <= 200 || coo[index_coo][3] - coo[index_coo][1] <= 200))
			{
				find_black_pixels_around(pixbuf, x + direction[i][0], y + direction[i][1], is_visited, index_coo, coo);
			}
		}
	}
}

int find_letter(GdkPixbuf *pixbuf, int **coo)
{
	int width = gdk_pixbuf_get_width(pixbuf);
    	int height = gdk_pixbuf_get_height(pixbuf);
    	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	
	int nb_letter = 0;

	int *is_visited = malloc(width * height * sizeof(int)); // 0 if False 1 if True
	memset(is_visited, 0, width * height * sizeof(int));
	
	int index_coo = 0;

	for(int y = 0; y < height; y ++)
	{
		for(int x = 0; x < width; x ++)
		{
			if(is_visited[y * width + x] == 0)
			{	
				guchar *p = pixels + y * rowstride + x * n_channels;
				
				if(p[0] == 255)
				{
					coo[index_coo][0] = coo[index_coo][2] = x;
					coo[index_coo][1] = coo[index_coo][3] = y;
					find_black_pixels_around(pixbuf, x, y, is_visited, index_coo, coo);
					is_visited[y * width + x] = 1;
					index_coo ++;
					nb_letter ++;
				}
			}
		}
	}
	
	free(is_visited);
	return nb_letter;
}

void generate_letter(GdkPixbuf *pixbuf, GdkPixbuf *pixbuf_to_crop, int **coo, char *output_file)
{
	int index_coo = 0;
	
	char* full_path = malloc(strlen(output_file) + 40 * sizeof(char));
	
	while(coo[index_coo][0] != 0)
	{
		if(coo[index_coo][0] < coo[index_coo][2] && coo[index_coo][1] < coo[index_coo][3] &&
				coo[index_coo][2] - coo[index_coo][0] <= 200 && coo[index_coo][3] - coo[index_coo][1] <= 200 &&
				coo[index_coo][2] - coo[index_coo][0] >= 5 && coo[index_coo][3] - coo[index_coo][1] >= 5)
		{
			GdkPixbuf *letter = crop(pixbuf_to_crop, coo[index_coo][0], coo[index_coo][1], coo[index_coo][2], coo[index_coo][3]);
			sprintf(full_path, "%s/%s%d_%d.png",output_file , output_file, coo[index_coo][0], coo[index_coo][1]);
			save_pixbuf_as_png(letter, full_path);
			g_object_unref(letter);
		}
		index_coo ++;
	}
	free(full_path);
}

void find_grid_and_words(int *grid_coo, int *word_coo, int **coo, int nb_letter)
{
	
	int *box1_coo = malloc(4 * sizeof(int));
	int *box2_coo = malloc(4 * sizeof(int));
        
	int box1 = 0;
	int box2 = 0;

	int seuil = 40;
	for(int i = 0; i < nb_letter; i ++)
	{
		if(box1 == 0)
		{
			box1_coo[0] = coo[i][0];
			box1_coo[1] = coo[i][1];
			box1_coo[2] = coo[i][2];
			box1_coo[3] = coo[i][3];
			box1 ++;
		}
		else if (!(coo[i][2] < box1_coo[0] - seuil || coo[i][0] > box1_coo[2] + seuil || 
      				coo[i][3] < box1_coo[1] - seuil || coo[i][1] > box1_coo[3] + seuil))
		{
			box1_coo[0] = (box1_coo[0] < coo[i][0]) ? box1_coo[0] : coo[i][0];
            		box1_coo[1] = (box1_coo[1] < coo[i][1]) ? box1_coo[1] : coo[i][1];
            		box1_coo[2] = (box1_coo[2] > coo[i][2]) ? box1_coo[2] : coo[i][2];
            		box1_coo[3] = (box1_coo[3] > coo[i][3]) ? box1_coo[3] : coo[i][3];
			box1 ++;
		}
		else if(box2 == 0)
		{
			box2_coo[0] = coo[i][0];
                        box2_coo[1] = coo[i][1];
                        box2_coo[2] = coo[i][2];
                        box2_coo[3] = coo[i][3];
                        box2 ++;
		}
		else if (!(coo[i][2] < box2_coo[0] - seuil || coo[i][0] > box2_coo[2] + seuil ||
                                coo[i][3] < box2_coo[1] - seuil || coo[i][1] > box2_coo[3] + seuil))
                {
                        box2_coo[0] = (box2_coo[0] < coo[i][0]) ? box2_coo[0] : coo[i][0];
            		box2_coo[1] = (box2_coo[1] < coo[i][1]) ? box2_coo[1] : coo[i][1];
           		box2_coo[2] = (box2_coo[2] > coo[i][2]) ? box2_coo[2] : coo[i][2];
            		box2_coo[3] = (box2_coo[3] > coo[i][3]) ? box2_coo[3] : coo[i][3];
                        box2 ++;
                }
	}
	if(box1 < box2) //box1 = words
	{
		word_coo[0] = box1_coo[0];
		word_coo[1] = box1_coo[1];
		word_coo[2] = box1_coo[2];
		word_coo[3] = box1_coo[3];
		grid_coo[0] = box2_coo[0];
                grid_coo[1] = box2_coo[1];
                grid_coo[2] = box2_coo[2];
                grid_coo[3] = box2_coo[3];
	}
	else //box2 = words
	{
		word_coo[0] = box2_coo[0];
                word_coo[1] = box2_coo[1];
                word_coo[2] = box2_coo[2];
                word_coo[3] = box2_coo[3];
                grid_coo[0] = box1_coo[0];
                grid_coo[1] = box1_coo[1];
                grid_coo[2] = box1_coo[2];
               	grid_coo[3] = box1_coo[3];
	}
	free(box1_coo);
	free(box2_coo);
}

void find_word_by_word(int **coo, int **word_list, int *words_coo, int nb_letter, int nb_words)
{
	int seuil = 20;
	int seuilx = 20;
	int seuily = 5;

	for(int i = 0; i < nb_letter; i ++)
	{
		if(!(coo[i][2] < words_coo[0] - seuil || coo[i][0] > words_coo[2] + seuil ||
                                coo[i][3] < words_coo[1] - seuil || coo[i][1] > words_coo[3] + seuil))
		{
			int found = 0;
			int index_word = 0;
			while(found == 0 && index_word < nb_words)
			{
				if(word_list[index_word][0] == 0 && word_list[index_word][2] == 0)
				{
					word_list[index_word][0] = coo[i][0];
					word_list[index_word][1] = coo[i][1];
					word_list[index_word][2] = coo[i][2];
					word_list[index_word][3] = coo[i][3];
					found = 1;
				}
				else if (!(coo[i][2] < word_list[index_word][0] - seuilx || coo[i][0] > word_list[index_word][2] + seuilx) && 
						!(coo[i][3] < word_list[index_word][1] - seuily || coo[i][1] > word_list[index_word][3] + seuily))
                		{
                        		word_list[index_word][0] = (word_list[index_word][0] < coo[i][0]) ? word_list[index_word][0] : coo[i][0];
                       			word_list[index_word][1] = (word_list[index_word][1] < coo[i][1]) ? word_list[index_word][1] : coo[i][1];
                        		word_list[index_word][2] = (word_list[index_word][2] > coo[i][2]) ? word_list[index_word][2] : coo[i][2];
                        		word_list[index_word][3] = (word_list[index_word][3] > coo[i][3]) ? word_list[index_word][3] : coo[i][3];
                			found = 1;
				}
				else
				{
					index_word ++;
				}
			}
		}
	}
}

void pipeline(char *filename, char *output_gw_file, char *output_letter_file)
{
	int nb_letter = 0;
	int nb_words = 50; // we state that there will not be more than 50 words in an exercise
	GdkPixbuf *pixbuf = load_image(filename);
	GdkPixbuf *pixbuf_to_slice = load_image(filename);
	int width = gdk_pixbuf_get_width(pixbuf);
    	int height = gdk_pixbuf_get_height(pixbuf);	
	int *grid_coo = malloc(4 * sizeof(int));
        int *words_coo = malloc(4 * sizeof(int));
	int **coo = malloc(width * height * sizeof(int*));
	for(int i = 0; i < width * height; i ++)
	{
		coo[i] = malloc(4 * sizeof(int)); // coo[i][0] = x1 coo[i][1] = y1 coo[i][2] = x2 coo[i][3] = y2
		coo[i][0] = coo[i][1] = coo[i][2] = coo[i][3] = 0;
	}

	convert_to_grayscale(pixbuf);
	binarize_image(pixbuf, 180);
	
	double angle = find_good_rotation(pixbuf);
	pixbuf = rotate_image(pixbuf, angle);
	pixbuf_to_slice = rotate_image(pixbuf_to_slice, angle);

	invert_color(pixbuf);

	nb_letter = find_letter(pixbuf, coo);
	
	generate_letter(pixbuf, pixbuf_to_slice, coo, output_letter_file);

	find_grid_and_words(grid_coo, words_coo, coo, nb_letter);

	int **word_list = malloc(nb_words * sizeof(int*));
	for(int i = 0; i < nb_words; i ++)
	{

		word_list[i] = malloc(4 * sizeof(int));
		word_list[i][0] = word_list[i][1] = word_list[i][2] = word_list[i][3] = 0;
	}

	find_word_by_word(coo, word_list, words_coo, nb_letter, nb_words);
	


	// crop the grid, word list and words of the image
	
	char* word_path = malloc(strlen(output_gw_file) + 40 * sizeof(char));
	for(int i = 0; i < nb_words; i ++)
	{
		if(word_list[i][2] - word_list[i][0] > 0 && word_list[i][3] - word_list[i][1] > 0)
		{
        		sprintf(word_path, "%s/word_n%i.png",output_gw_file, i);
			GdkPixbuf *word = crop(pixbuf_to_slice, word_list[i][0], word_list[i][1], word_list[i][2], word_list[i][3]);
        		save_pixbuf_as_png(word, word_path);
        		g_object_unref(word);
		}
	}

	char* grid_path = malloc(strlen(output_gw_file) + 40 * sizeof(char));
	char* words_path = malloc(strlen(output_gw_file) + 40 * sizeof(char));
	sprintf(grid_path, "%s/grid.png",output_gw_file);
	sprintf(words_path, "%s/words.png",output_gw_file);
	
	GdkPixbuf *grid = crop(pixbuf_to_slice, grid_coo[0], grid_coo[1], grid_coo[2], grid_coo[3]);
        save_pixbuf_as_png(grid, grid_path);
        g_object_unref(grid);
	
        GdkPixbuf *words = crop(pixbuf_to_slice, words_coo[0], words_coo[1], words_coo[2], words_coo[3]);
        save_pixbuf_as_png(words, words_path);
        g_object_unref(words);
	
	
	
	// free all pointers
	
	for(int i = 0; i < width * height; i ++)
	{
		free(coo[i]);
	}
	for(int i = 0; i < nb_words; i ++)
	{
		free(word_list[i]);
	}
	free(coo);
	free(word_list);
	free(grid_coo);
	free(words_coo);
	free(grid_path);
	free(word_path);
	g_object_unref(pixbuf);
	g_object_unref(pixbuf_to_slice);
}

void main(int argc, char *argv[])
{
	pipeline(argv[1], argv[2], argv[3]);
}

