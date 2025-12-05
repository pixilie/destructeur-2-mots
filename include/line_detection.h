#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
    int grid_coo[4];
    int words_coo[4];
    int nb_letters;
    int nb_letters_grid;
    int nb_letters_words;
    int nb_words;
} pipelineResult;

void sobel_filter(GdkPixbuf *pixbuf);
void invert_color(GdkPixbuf *pixbuf);
double find_good_rotation(GdkPixbuf *pixbuf);
void remove_lines(GdkPixbuf *pixbuf);
void find_black_pixels_around(GdkPixbuf *pixbuf, int x, int y, int *is_visited,
                              int index_coo, int **coo);
int find_letter(GdkPixbuf *pixbuf, int **coo);
void generate_letter(GdkPixbuf *pixbuf_to_crop, int **coo, int *grid_coo,
                     int *words_coo, char *output_file,
                     int *nb_letters_grid_out, int *nb_letters_words_out);
void find_grid_and_words(int *grid_coo, int *word_coo, int **coo,
                         int nb_letter);
int find_word_by_word(int **coo, int **word_list, int *words_coo, int nb_letter,
                      int nb_words);
pipelineResult pipeline(char *filename, char *output_gw_file,
                        char *output_letter_file);
