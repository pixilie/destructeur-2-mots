#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void convert_to_grayscale(GdkPixbuf *pixbuf);
int calculate_mean_treshold(GdkPixbuf *pixbuf);
void binarize_image(GdkPixbuf *pixbuf, int treshold);
int convert_to_black_and_white(GdkPixbuf *pixbuf);
int *create_histogram(GdkPixbuf *pixbuf);
void print_histogram(GdkPixbuf *pixbuf);

int find_minimum_index(int start_index, guchar neighborhood[9]);
void selection_sort(guchar neighborhood[9]);
void median_filter_3x3(GdkPixbuf *pixbuf);
