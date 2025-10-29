#define _POSIX_C_SOURCE 200809L

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
 *
 * Notes:
 *  - The returned pointer points to a static buffer; do not free it.
 *  - Uses readlink("/proc/self/exe") and dirname() to locate the executable.
 *  - PATH_MAX is used to size the buffer.
 */
char *get_image_path(const char *filename)
{
    static char image_path[PATH_MAX];

    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1)
    {
        g_printerr("ERROR: Failed to read binary path\n");
        return NULL;
    }
    exe_path[len] = '\0';

    char *dir = dirname(exe_path);

    snprintf(image_path, sizeof(image_path), "%s/../assets/%s", dir, filename);

    return image_path;
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
 *
 * Side effects:
 *  - Prints a failure message to stdout/stderr on error.
 *  - Frees any GError allocated by gdk_pixbuf_new_from_file.
 *
 * Notes:
 *  - This function delegates path construction to get_image_path().
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
 *
 * Side effects:
 *  - Prints an error message to stderr if saving fails.
 *  - Frees any GError allocated by gdk_pixbuf_save.
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
