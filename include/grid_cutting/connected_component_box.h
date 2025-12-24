#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void find_white_pixels_around(GdkPixbuf *pixbuf, int start_x, int start_y,
                              int *is_visited, int index_coo, int **coo);
