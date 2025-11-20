#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

double compute_projection_variance(GdkPixbuf *pix);
double detect_best_angle(GdkPixbuf *src);
