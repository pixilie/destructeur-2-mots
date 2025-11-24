#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

GdkPixbuf *rotate_image(GdkPixbuf *pixbuf, double angle_degrees);
GdkPixbuf *downscale_pixbuf(GdkPixbuf *pixbuf, int target_width);
double compute_projection_variance(GdkPixbuf *pixbuf);
double detect_best_angle(GdkPixbuf *pixbuf);
GdkPixbuf *rotate_image_automatic(GdkPixbuf *pixbuf);
