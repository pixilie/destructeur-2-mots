#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

GdkPixbuf *rotate_image(GdkPixbuf *src, double angle_degrees);
double compute_projection_variance(GdkPixbuf *pix);
double detect_best_angle(GdkPixbuf *src);

