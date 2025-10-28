#pragma once

#include <gtk/gtk.h>

char* get_image_path(const char* filename); //Return absolute path of image in assets
void convert_to_grayscale(GdkPixbuf *pixbuf); //Convert colored image (RGB or RGBA) in grayscale
void binarize_image(GdkPixbuf *pixbuf, int treshold); //Convert grayscale image in black and white
void slice_from(const char *image_path, int x, int y, int direction);
void slice_in_n(const char *image_path, int n_slice);
static void on_activate(GtkApplication *app); 
