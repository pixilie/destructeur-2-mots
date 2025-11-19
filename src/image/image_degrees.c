#define _GNU_SOURCE

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <string.h>

#include "image_rotation.h"


double compute_projection_variance(GdkPixbuf *pix) {
//used to calculate the variance of an image
//the higher the return is, the better is the angle of the image

    int w = gdk_pixbuf_get_width(pix);
    int h = gdk_pixbuf_get_height(pix);
    int stride = gdk_pixbuf_get_rowstride(pix);
    guchar *px = gdk_pixbuf_get_pixels(pix);
    int nch = gdk_pixbuf_get_n_channels(pix);

    double sum = 0.0;
    double sum2 = 0.0;

    //iterate through each line
    for(int y = 0; y < h; y++) {
        int line_sum = 0;
        guchar *row = px + y * stride;

        for(int x = 0; x < w; x++) {
            line_sum += (255 - row[x * nch]);
        }

        sum += line_sum;
        sum2 += line_sum * line_sum;
    }

    double mean = sum / h;
    double var = sum2 / h - mean * mean;
    return var;
}

double detect_best_angle(GdkPixbuf *src) {
//test image from -10° to 10°
//it NEED to take a binary-image (white/black) to function correctly
//return a double which is the better angle found

    double best_angle = 0.0;
    double best_score = -1.0;

    //check every angle from 0 to 20 (200tests)
    for(double angle = 0.0; angle < 20.0; angle += 0.1) {

        GdkPixbuf * temp = rotate_image(src, angle);
        double score = compute_projection_variance(temp);
        g_object_unref(temp); //maybe free(temp) ?

        if(score > best_score) {
            best_score = score;
            best_angle = angle;
        }
    }

    return best_angle;
}