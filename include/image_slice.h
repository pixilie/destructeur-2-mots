#pragma once

#include "gdk-pixbuf/gdk-pixbuf.h"

GdkPixbuf **slice_from(GdkPixbuf *pixbuf, int x, int y, int direction);
GdkPixbuf **slice_in_n(GdkPixbuf *pixbuf, int n_slice);
