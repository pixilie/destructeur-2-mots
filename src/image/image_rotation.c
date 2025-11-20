#define _GNU_SOURCE

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <string.h>

/**
 * rotate_image:
 * Rotate a GdkPixbuf by a given angle (degrees) and return a newly allocated
 * GdkPixbuf containing the rotated image.
 *
 * Parameters:
 *  - src          : pointer to the source GdkPixbuf to rotate (not modified).
 *  - angle_degrees: rotation angle in degrees (positive = clockwise).
 *
 * Returns:
 *  - A newly-allocated GdkPixbuf* containing the rotated image. The caller
 *    owns the returned pixbuf and must call g_object_unref() when done.
 *  - The function does not return NULL on error in the current implementation;
 *    callers should still validate the return value.
 */
GdkPixbuf *rotate_image(GdkPixbuf *src, double angle_degrees)
{
    int width = gdk_pixbuf_get_width(src);
    int height = gdk_pixbuf_get_height(src);
    int rowstride = gdk_pixbuf_get_rowstride(src);
    int n_channels = gdk_pixbuf_get_n_channels(src);
    guchar *pixels = gdk_pixbuf_get_pixels(src);

    // Convert degrees to radians
    double angle = angle_degrees * M_PI / 180;

    int new_width = (int)(fabs(width * cos(angle)) + fabs(height * sin(angle)));
    int new_height =
        (int)(fabs(width * sin(angle)) + fabs(height * cos(angle)));

    // Create new Pixbuf for the rotated image
    GdkPixbuf *new =
        gdk_pixbuf_new(GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha(src), 8,
                       new_width, new_height);

    int new_rowstride = gdk_pixbuf_get_rowstride(new);
    int new_n_channels = gdk_pixbuf_get_n_channels(new);
    guchar *new_pixels = gdk_pixbuf_get_pixels(new);

    // Fill new Pixbuf with black backrgound
    memset(new_pixels, 0, new_rowstride * new_height);

    double center_x = width / 2.0;
    double center_y = height / 2.0;
    double new_center_x = new_width / 2.0;
    double new_center_y = new_height / 2.0;

    double cos_a = cos(angle);
    double sin_a = sin(angle);

    for (int new_y = 0; new_y < new_height; new_y++)
    {
        for (int new_x = 0; new_x < new_width; new_x++)
        {
            // Find location of pixel in original image
            double x = (new_x - new_center_x) * cos_a +
                       (new_y - new_center_y) * sin_a + center_x;
            double y = -(new_x - new_center_x) * sin_a +
                       (new_y - new_center_y) * cos_a + center_y;

            // Copy pixel color if it's inside bounds
            if (x >= 0 && x < width && y >= 0 && y < height)
            {
                int x_i = (int)x;
                int y_i = (int)y;

                guchar *pixel = pixels + y_i * rowstride + x_i * n_channels;
                guchar *new_pixel =
                    new_pixels + new_y * new_rowstride + new_x * new_n_channels;

                // Copy pixel (RGB or RGBA)
                for (int c = 0; c < n_channels; c++)
                {
                    new_pixel[c] = pixel[c];
                }
            }
        }
    }
    return new;
}

double compute_projection_variance(GdkPixbuf *pix)
{
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
    for(int y = 0; y < h; y++)
    {
        int line_sum = 0;
        guchar *row = px + y * stride;

        for(int x = 0; x < w; x++)
        {
            line_sum += (255 - row[x * nch]);
        }

        sum += line_sum;
        sum2 += line_sum * line_sum;
    }

    double mean = sum / h;
    double var = sum2 / h - mean * mean;
    return var;
}

double detect_best_angle(GdkPixbuf *src)
{
//test image from -10° to 10°
//it NEED to take a binary-image (white/black) to function correctly
//return a double which is the better angle found

    double best_angle = 0.0;
    double best_score = -1.0;

    //check every angle from 0 to 20 (200tests)
    for(double angle = 0.0; angle < 20.0; angle += 0.1)
    {

        GdkPixbuf * temp = rotate_image(src, angle);
        double score = compute_projection_variance(temp);
        g_object_unref(temp); //maybe free(temp) ?

        if(score > best_score)
        {
            best_score = score;
            best_angle = angle;
        }
    }

    return best_angle;
}
