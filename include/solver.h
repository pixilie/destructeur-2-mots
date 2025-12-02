#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
} GridLetter;

int len_word(char word[]);
void solve(int rows, int cols, char tab[rows][cols], char word[], int *x1,
           int *y1, int *x2, int *y2);

int center_x(const GridLetter *grid_letter);
int center_y(const GridLetter *grid_letter);
int compare_x(const void *letter1, const void *letter2);
int compare_y(const void *letter1, const void *letter2);
GridLetter **build_grid_from_image(GridLetter *letters, int nb_letters,
                                   int *rows_out, int *cols_out);
char **build_grid_array(GdkPixbuf *pixbuf, GridLetter **grid_letters, int rows,
                        int cols);
