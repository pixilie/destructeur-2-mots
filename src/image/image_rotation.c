#define _GNU_SOURCE

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
