#pragma once

#include "../neural_network.h"
#include "letter.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

Letter **build_grid_from_image(Letter *grid_letters, int nb_letters,
                               int *rows_out, int *cols_out);
char **build_grid_array(NeuralNetwork *nn, GdkPixbuf *pixbuf,
                        Letter **grid_letters, int rows, int cols,
                        int *rows_out, int *cols_out);
