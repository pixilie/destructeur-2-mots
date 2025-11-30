#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void draw_pixel(GdkPixbuf *pixbuf, int x, int y, int thickness);
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness);
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int x3,
                    int y3, int x4, int y4, int thickness);

int **get_all_words_coordinates(int rows, int cols, char tab[rows][cols],
                                int words_count, char **words);

int* get_word_image_coordinates(int grid_coos[4], int rows, int cols, int x1,
                                  int y1, int x2, int y2);

void solve_grid();
