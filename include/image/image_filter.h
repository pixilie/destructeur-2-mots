#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void erode_3x3(GdkPixbuf *pixbuf);
void dilate_3x3(GdkPixbuf *pixbuf);

int find_minimum_index(int start_index, guchar neighborhood[9]);
void selection_sort(guchar neighborhood[9]);
void median_filter_3x3(GdkPixbuf *pixbuf);
