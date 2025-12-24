#pragma once

#include "../solver/letter.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

int find_letter(GdkPixbuf *pixbuf, int **coo);
void generate_letter(int *grid_coo, int *words_coo, int **coo, int nb_letters,
                     int *nb_grid_letters_out, int *nb_words_letters_out,
                     Letter **grid_letters_out, Letter **words_letters_out);
