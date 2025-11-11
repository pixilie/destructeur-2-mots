#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void convert_to_grayscale(GdkPixbuf *pixbuf);
int calculate_mean_treshold(GdkPixbuf *pixbuf);
void binarize_image(GdkPixbuf *pixbuf, int treshold);
