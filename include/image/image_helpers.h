#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

char *get_image_path(const char *filename);
GdkPixbuf *load_image(char *filename);
int save_pixbuf_as_png(GdkPixbuf *pixbuf, const char *filename);
GdkPixbuf *scale_pixbuf_to_28x28(GdkPixbuf *src);
void pixbuf_to_input_vector(GdkPixbuf *pixbuf, double *out);
