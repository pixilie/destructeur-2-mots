#define _GNU_SOURCE

#include "../../include/image_processing/rotation.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>

/*
 * rotate_image:
 * Rotate a GdkPixbuf by a given angle (degrees) and return a new GdkPixbuf.
 * Caller owns the returned pixbuf and must g_object_unref() it.
 *
 * Parameters:
 *  - pixbuf: source image (not modified)
 *  - angle_degrees: rotation angle in degrees (positive = clockwise)
 */
GdkPixbuf *rotate_image(GdkPixbuf *pixbuf, double angle_degrees)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Convert degrees to radians
    double angle = angle_degrees * M_PI / 180;

    int new_width = (int)(fabs(width * cos(angle)) + fabs(height * sin(angle)));
    int new_height =
        (int)(fabs(width * sin(angle)) + fabs(height * cos(angle)));

    // Create new Pixbuf for the rotated image
    GdkPixbuf *new =
        gdk_pixbuf_new(GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha(pixbuf), 8,
                       new_width, new_height);

    int new_rowstride = gdk_pixbuf_get_rowstride(new);
    int new_n_channels = gdk_pixbuf_get_n_channels(new);
    guchar *new_pixels = gdk_pixbuf_get_pixels(new);

    // Fill new Pixbuf with white backrgound
    memset(new_pixels, 255, new_rowstride * new_height);

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
