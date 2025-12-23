#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

int calculate_mean_treshold(GdkPixbuf *pixbuf);

int *create_histogram(GdkPixbuf *pixbuf);
int calculate_otsu_threshold(GdkPixbuf *pixbuf);

void binarize_image(GdkPixbuf *pixbuf, int threshold);
int convert_to_black_and_white(GdkPixbuf *pixbuf);
