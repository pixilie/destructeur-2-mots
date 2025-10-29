#pragma once

#include "gdk-pixbuf/gdk-pixbuf.h"

GdkPixbuf **slice_from(GdkPixbuf *pixbuf, int x, int y, int direction);
GdkPixbuf **slice_in_n(GdkPixbuf *pixbuf, int n_slice);
GdkPixbuf* crop(GdkPixbuf *src, int x1, int y1, int x2, int y2);
