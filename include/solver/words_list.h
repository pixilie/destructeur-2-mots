#pragma once

#include "../neural_network.h"
#include "letter.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

Letter **build_words_list_from_image(Letter *words_letters, int nb_letters,
                                     int **words_size_out,
                                     int *words_count_out);
char **build_words_list(NeuralNetwork *nn, GdkPixbuf *pixbuf,
                        Letter **words_letters, int nb_words, int *words_size);
