#define _GNU_SOURCE

#include "../../include/image_processing/rotation.h"
#include "../../include/image_processing/automatic_rotation.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>

/*
 * downscale_pixbuf:
 * Downscale pixbuf to the given target width (preserving aspect ratio).
 * Returns the downscaled pixbuf or the original when already small enough.
 *
 * Parameters:
 *  - pixbuf: source image
 *  - target_width: desired maximum width
 */
GdkPixbuf *downscale_pixbuf(GdkPixbuf *pixbuf, int target_width)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    if (width <= target_width)
    {
        return pixbuf; // Image already small enough -> No downscaling needed
    }

    double scale = (double)target_width / width;
    int new_width = target_width;
    int new_height = (int)height * scale;

    // Return the downscaled pixbuf
    return gdk_pixbuf_scale_simple(pixbuf, new_width, new_height,
                                   GDK_INTERP_NEAREST);
}

/*
 * compute_projection_variance:
 * Compute a variance-based score for the image projection. Higher values
 * indicate stronger line structure useful for angle selection.
 *
 * Parameters:
 *  - pixbuf: input image (usually downscaled, B/W)
 */
double compute_projection_variance(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    double row_sums[height];

    // Iterate through each line
    for (int y = 0; y < height; y++)
    {
        double line_sum = 0;
        guchar *row = pixels + y * rowstride;

        for (int x = 1; x < width; x++)
        {
            line_sum += fabs((double)row[x * n_channels] -
                             (double)row[(x - 1) * n_channels]);
        }

        row_sums[y] = line_sum;
    }

    double mean = 0;
    for (int y = 0; y < height; y++)
    {
        mean += row_sums[y];
    }
    mean /= height;

    double var = 0;
    for (int y = 0; y < height; y++)
    {
        var += (row_sums[y] - mean) * (row_sums[y] - mean);
    }
    var /= height;

    return var;
}

/*
 * detect_best_angle:
 * Search candidate rotation angles and return the best angle (degrees).
 * The function evaluates rotated images and picks the angle with highest score.
 * Works best on binarized/black-and-white input.
 */
double detect_best_angle(GdkPixbuf *pixbuf)
{

    // Downscale to speed up rotation to 100 pixels width (less pixels)
    GdkPixbuf *downscaled_pixbuf = downscale_pixbuf(pixbuf, 150);

    double best_angle = 0.0;
    double best_score = -1.0;

    // First search : check every 5 degrees from -45° to 45° (= 19 tests)
    for (double angle = -45.0; angle <= 45.0; angle += 5.0)
    {
        GdkPixbuf *rotated_pixbuf = rotate_image(downscaled_pixbuf, angle);
        double score = compute_projection_variance(rotated_pixbuf);
        g_object_unref(rotated_pixbuf);

        if (score > best_score)
        {
            best_score = score;
            best_angle = angle; // New best score found -> New best angle found
        }
    }

    // More refined second search : check every 0.1 degree around the previous
    // best angle (60 tests)
    double search_start = best_angle - 3.0;
    double search_end = best_angle + 3.0;
    for (double angle = search_start; angle <= search_end; angle += 0.1)
    {
        GdkPixbuf *rotated_pixbuf = rotate_image(downscaled_pixbuf, angle);
        double score = compute_projection_variance(rotated_pixbuf);
        g_object_unref(rotated_pixbuf);

        if (score > best_score)
        {
            best_score = score;
            best_angle = angle; // New best score found -> New best angle found
        }
    }

    if (downscaled_pixbuf)
        g_object_unref(downscaled_pixbuf);

    if (best_angle > -1.0 &&
        best_angle < 1.0) // Avoid small angles for straight images
    {
        return 0.0;
    }

    return best_angle;
}

/*
 * rotate_image_automatic:
 * Detect the best rotation angle and rotate the image accordingly.
 * Returns the rotated pixbuf (new reference) or the original if no rotation
 * needed.
 */
GdkPixbuf *rotate_image_automatic(GdkPixbuf *pixbuf)
{
    double best_angle = detect_best_angle(pixbuf);
    if (best_angle == 0)
    {
        printf("Image is already upright, no rotation needed\n");
        return pixbuf;
    }
    printf("Image automatically rotated by best rotation angle : %.2f°\n",
           best_angle);
    return rotate_image(pixbuf, best_angle);
}
