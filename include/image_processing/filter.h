#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void erode_3x3(GdkPixbuf *pixbuf);
void dilate_3x3(GdkPixbuf *pixbuf);

int find_minimum_index(int start_index, guchar *neighborhood);
void selection_sort(guchar *neighborhood);

void filter_neighborhood_3x3(GdkPixbuf *pixbuf, GdkPixbuf *copy, int x, int y,
                             int width, int height, int rowstride,
                             int n_channels);
void median_filter_3x3(GdkPixbuf *pixbuf);
