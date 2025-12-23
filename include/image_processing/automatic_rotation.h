#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

GdkPixbuf *downscale_pixbuf(GdkPixbuf *pixbuf, int target_width);

double compute_projection_variance(GdkPixbuf *pixbuf);
double detect_best_angle(GdkPixbuf *pixbuf);

GdkPixbuf *rotate_image_automatic(GdkPixbuf *pixbuf);
