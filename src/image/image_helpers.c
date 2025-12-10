#define _XOPEN_SOURCE 700

#include "../include/image/image_helpers.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * get_image_path:
 * Build an absolute path to an asset located in the project's ../assets
 * directory relative to the running executable.
 *
 * Parameters:
 *  - filename: name of the asset file (e.g. "image.png").
 *
 * Returns:
 *  - Pointer to a static buffer containing the constructed path on success.
 *  - NULL on failure (for example if /proc/self/exe cannot be read).
 */
char *get_image_path(const char *filename)
{
    static char image_path[PATH_MAX];
    char resolved_path[PATH_MAX];

    if (g_path_is_absolute(filename))
    {
        snprintf(image_path, sizeof(image_path), "%s", filename);
        return image_path;
    }

    if (realpath(filename, resolved_path))
    {
        snprintf(image_path, sizeof(image_path), "%s", resolved_path);
        return image_path;
    }

    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1)
    {
        g_printerr("ERROR: Failed to read binary path\n");
        return NULL;
    }
    exe_path[len] = '\0';

    char *dir = dirname(exe_path);

    snprintf(resolved_path, sizeof(resolved_path), "%s/../assets/%s", dir,
             filename);
    if (realpath(resolved_path, image_path))
    {
        return image_path;
    }

    g_printerr("ERROR: Could not resolve path for '%s'\n", filename);
    return NULL;
}

/**
 * load_image:
 * Load an image file from the assets directory into a GdkPixbuf.
 *
 * Parameters:
 *  - filename: asset filename (relative name passed to get_image_path).
 *
 * Returns:
 *  - Newly-allocated GdkPixbuf* on success (caller must unref when done).
 *  - NULL on failure.
 */
GdkPixbuf *load_image(char *filename)
{
    GError *error = NULL;
    const char *path = get_image_path(filename);

    if (!path)
    {
        printf("[FAIL] Failed to load image\n");
        return 0;
    }

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, &error);

    if (pixbuf == NULL)
    {
        printf("[FAIL] Failed to load image : %s\n", error->message);
        if (error)
        {
            g_error_free(error);
        }
        return 0;
    }

    return pixbuf;
}

/**
 * save_pixbuf_as_png:
 * Save a GdkPixbuf to a PNG file.
 *
 * Parameters:
 *  - pixbuf: pointer to a valid GdkPixbuf to save.
 *  - filename: destination filename (path on the filesystem).
 *
 * Returns:
 *  - 1 on success.
 *  - 0 on failure (invalid arguments or save error).
 */
int save_pixbuf_as_png(GdkPixbuf *pixbuf, const char *filename)
{
    if (!pixbuf || !filename)
        return 0;

    GError *error = NULL;

    if (!gdk_pixbuf_save(pixbuf, filename, "png", &error, NULL))
    {
        fprintf(stderr, "Error saving file '%s': %s\n", filename,
                error ? error->message : "unknown error");
        if (error)
            g_error_free(error);
        return 0;
    }

    return 1;
}

/**
 * Resizes a GdkPixbuf to a fixed 28x28 size while preserving the aspect ratio.
 *
 * Parameters:
 * - src : the source GdkPixbuf to be scaled (must be valid)
 *
 * Returns:
 * A new GdkPixbuf of size 28x28 containing the centered, scaled image.
 * Returns NULL if src is invalid or memory allocation fails.
 */
GdkPixbuf *scale_pixbuf_to_28x28(GdkPixbuf *src)
{
    if (!src)
    {
        return NULL;
    }

    int src_w = gdk_pixbuf_get_width(src);
    int src_h = gdk_pixbuf_get_height(src);

    GdkPixbuf *final_pixbuf =
        gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 28, 28);

    gdk_pixbuf_fill(final_pixbuf, 0xFFFFFFFF);

    int target_size = 20;

    double scale_x = (double)target_size / src_w;
    double scale_y = (double)target_size / src_h;
    double scale_factor = (scale_x < scale_y) ? scale_x : scale_y;

    int new_w = (int)(src_w * scale_factor);
    int new_h = (int)(src_h * scale_factor);

    if (new_w < 1)
    {
        new_w = 1;
    }
    if (new_h < 1)
    {
        new_h = 1;
    }

    GdkPixbuf *scaled_src =
        gdk_pixbuf_scale_simple(src, new_w, new_h, GDK_INTERP_HYPER);

    if (!scaled_src)
    {
        g_object_unref(final_pixbuf);
        return NULL;
    }

    int offset_x = (28 - new_w) / 2;
    int offset_y = (28 - new_h) / 2;

    gdk_pixbuf_copy_area(scaled_src, 0, 0, new_w, new_h, final_pixbuf, offset_x,
                         offset_y);

    g_object_unref(scaled_src);

    return final_pixbuf;
}

/**
 * Convert a GdkPixbuf into a normalized input vector for the neural network.
 * Performs normalization (0.0 - 1.0) and color inversion (assuming input is
 * black text on white).
 *
 * Parameters:
 * - pixbuf : the source image (must be valid)
 * - out    : pointer to an allocated double array (size must be width * height)
 */
void pixbuf_to_input_vector(GdkPixbuf *pixbuf, double *out)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int y = 0; y < height; y++)
    {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar pixel = row[x * n_channels];
            out[y * width + x] = (double)pixel / 255.0;
        }
    }
}
