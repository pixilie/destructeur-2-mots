#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "solver.h"

typedef struct
{
    int nb_rows;
    int nb_cols;
    char **grid;
} Grid;

typedef struct
{
    int grid_coo[4];
    int words_coo[4];
    int nb_letters;
    int nb_letters_grid;
    int nb_letters_words;
    int nb_words;

    Grid grid;
    int **words;
} PipelineResult;

void sobel_filter(GdkPixbuf *pixbuf);
void invert_color(GdkPixbuf *pixbuf);
double find_good_rotation(GdkPixbuf *pixbuf);
void remove_lines(GdkPixbuf *pixbuf);
void find_black_pixels_around(GdkPixbuf *pixbuf, int x, int y, int *is_visited,
                              int index_coo, int **coo);
int find_letter(GdkPixbuf *pixbuf, int **coo);
GridLetter* generate_letter(GdkPixbuf *pixbuf_to_crop, int *grid_coo, int **coo, char *output_file, int nb_letters);
void find_grid_and_words(int *grid_coo, int *word_coo, int **coo,
                         int nb_letter);
int find_word_by_word(int **coo, int **word_list, int *words_coo,
                       int nb_letter, int nb_words);
PipelineResult pipeline(char *filename, char *output_gw_file, char *output_letter_file);
