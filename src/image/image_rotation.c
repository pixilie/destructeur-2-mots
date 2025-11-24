#define _GNU_SOURCE

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>

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

GdkPixbuf *downscale_pixbuf(GdkPixbuf *pixbuf, int target_width)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    if (width <= target_width)
    {
        return pixbuf; // Image already small enough -> No downscaling needed
    }

    double scale = (double) target_width / width;
    int new_width = target_width;
    int new_height = (int) height * scale;

    // Return the downscaled pixbuf
    return gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_NEAREST);
}

double compute_projection_variance(GdkPixbuf *pixbuf)
{
    //used to calculate the variance of an image
    //the higher the return is, the better is the angle of the image
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    double sum = 0.0;
    double sum2 = 0.0;

    //iterate through each line
    for(int y = 0; y < height; y++)
    {
        int line_sum = 0;
        guchar *row = pixels + y * rowstride;

        for(int x = 1; x < width; x++)
        {
            int current_pixel = (int) row[x * n_channels];
            int previous_pixel = (int) row[(x - 1) * n_channels];
            
            // Measure the change between neighboring pixels
            int edge_strength = abs(current_pixel - previous_pixel);
            
            line_sum += edge_strength;
        }

        sum += line_sum;
        sum2 += line_sum * line_sum;
    }

    double mean = sum / height;
    double var = sum2 / height - mean * mean;
    return var;
}

double detect_best_angle(GdkPixbuf *pixbuf)
{
    //test image from -90° to 90°
    //it NEEDS to take a black and white image to function correctly
    //return a double which is the better angle found

    // Downscale to speed up rotation to 100 pixels width (less pixels)
    GdkPixbuf *downscaled_pixbuf = downscale_pixbuf(pixbuf, 100);

    double best_angle = 0.0;
    double best_score = -1.0;

    //check every angle from -90° to 90° (= 180 tests)
    for(double angle = -90.0; angle <= 90.0; angle += 1)
    {
        GdkPixbuf *rotated_pixbuf = rotate_image(downscaled_pixbuf, angle);
        double score = compute_projection_variance(rotated_pixbuf);
        g_object_unref(rotated_pixbuf);

        if(score > best_score)
        {
            best_score = score;
            best_angle = angle; // New best score found -> New best angle found
        }
    }

    return best_angle;
}

// Detects the best rotation angle and rotates the image by that angle
GdkPixbuf *rotate_image_automatic(GdkPixbuf *pixbuf)
{
    double best_angle = detect_best_angle(pixbuf);
    printf("Image automatically rotated by best rotation angle : %f\n", best_angle);
    return rotate_image(pixbuf, best_angle);
}
