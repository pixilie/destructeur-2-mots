#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void sobel_filter(GdkPixbuf *pixbuf);
void invert_color(GdkPixbuf *pixbuf);
double find_good_rotation(GdkPixbuf *pixbuf);
void remove_lines(GdkPixbuf *pixbuf);
static void find_black_pixels_around(GdkPixbuf *pixbuf, int x, int y,
                                     int *is_visited, int index_coo, int **coo);
void find_letter(GdkPixbuf *pixbuf, int **coo);
void generate_letter(GdkPixbuf *pixbuf_to_crop, int **coo, char *output_file);
void pipeline(char *filename, char *output_file);
