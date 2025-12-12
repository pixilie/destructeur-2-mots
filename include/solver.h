#pragma once

#include "neural_network.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
} Letter;

int len_word(char word[]);
void solve(int rows, int cols, char **grid, char word[], int *x1,
           int *y1, int *x2, int *y2);

int center_x(const Letter *letter);
int center_y(const Letter *letter);
int compare_x(const void *letter1, const void *letter2);
int compare_y(const void *letter1, const void *letter2);

Letter **build_grid_from_image(Letter *grid_letters, int nb_letters,
                                   int *rows_out, int *cols_out);
char **build_grid_array(GdkPixbuf *pixbuf, Letter **grid_letters, int rows,
                        int cols, int *rows_out, int *cols_out, NeuralNetwork *nn);

Letter **build_words_list_from_image(Letter *words_letters, int nb_letters,
                               int **words_size_out, int *words_count_out);
char **build_words_list(GdkPixbuf *pixbuf, Letter **words_letters, int nb_words, int *words_size, NeuralNetwork *nn);

int **get_solved_words_grid_coos(char **words, int words_count, char **grid, int rows, int cols);
int **get_solved_words_image_coos_drawing(int **words_grid_coos, int words_count, int grid_coos[4], int rows, int cols);
