#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

char *get_image_path(const char *filename);
GdkPixbuf *load_image(char *filename);
int save_pixbuf_as_png(GdkPixbuf *pixbuf, const char *filename);
