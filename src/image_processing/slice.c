#include "../../include/image_processing/slice.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * slice_from:
 * Split a pixbuf into two subpixbufs using a horizontal or vertical cut.
 *
 * Parameters:
 *  - pixbuf   : source GdkPixbuf to slice (must not be NULL).
 *  - x        : x coordinate for vertical cut.
 *  - y        : y coordinate for horizontal cut.
 *  - direction: 1 = horizontal (top/bottom), otherwise vertical (left/right).
 *
 * Returns:
 *  - Newly-allocated array of two GdkPixbuf* on success (caller must
 *    g_object_unref() each pixbuf and free() the array), or NULL on error.
 */
GdkPixbuf **slice_from(GdkPixbuf *pixbuf, int x, int y, int direction)
{
    if (!pixbuf)
        return NULL;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    GdkPixbuf **result = malloc(2 * sizeof(GdkPixbuf *));
    if (!result)
        return NULL;

    if (direction == 1) // horizontal cut
    {
        if (y <= 0 || y >= height)
        {
            free(result);
            return NULL;
        }

        int new_height = height - y;
        result[0] = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, width, y); // top
        result[1] =
            gdk_pixbuf_new_subpixbuf(pixbuf, 0, y, width, new_height); // bottom
    }
    else // vertical cut
    {
        if (x <= 0 || x >= width)
        {
            free(result);
            return NULL;
        }

        int new_width = width - x;
        result[0] = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, x, height); // left
        result[1] =
            gdk_pixbuf_new_subpixbuf(pixbuf, x, 0, new_width, height); // right
    }

    return result;
}

/*
 * slice_in_n:
 * Split a pixbuf into a grid of equal tiles (n_slice x n_slice).
 *
 * Parameters:
 *  - pixbuf  : source GdkPixbuf (must not be NULL).
 *  - n_slice : number of slices per row/column (must be > 0).
 *
 * Returns:
 *  - Array of n_slice*n_slice GdkPixbuf* in row-major order on success
 *    (caller must g_object_unref() each tile and free() the array), or NULL.
 */
GdkPixbuf **slice_in_n(GdkPixbuf *pixbuf, int n_slice) // FIX
{
    if (!pixbuf || n_slice <= 0)
        return NULL;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    int tile_width = width / n_slice;
    int tile_height = height / n_slice;
    int total = n_slice * n_slice;

    GdkPixbuf **tiles = malloc(total * sizeof(GdkPixbuf *));
    if (!tiles)
        return NULL;

    int index = 0;
    for (int i = 0; i < n_slice; i++)
    {
        for (int j = 0; j < n_slice; j++)
        {
            int x = j * tile_width;
            int y = i * tile_height;

            tiles[index++] =
                gdk_pixbuf_new_subpixbuf(pixbuf, x, y, tile_width, tile_height);
        }
    }

    return tiles;
}

/*
 * crop:
 * Return a subpixbuf for the rectangular region defined by (x1,y1)-(x2,y2).
 *
 * Parameters:
 *  - pixbuf : source GdkPixbuf (must not be NULL).
 *  - x1,y1  : coordinates of one corner of the crop rectangle.
 *  - x2,y2  : coordinates of the opposite corner.
 *
 * Returns:
 *  - GdkPixbuf* for the cropped region (caller must g_object_unref()), or NULL
 *    on error (invalid/out-of-bounds coordinates).
 */
GdkPixbuf *crop(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2)
{
    if (x2 < x1)
    {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y2 < y1)
    {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    int image_width = gdk_pixbuf_get_width(pixbuf);
    int image_height = gdk_pixbuf_get_height(pixbuf);

    x1 = CLAMP(x1, 0, image_width - 1);
    y1 = CLAMP(y1, 0, image_height - 1);
    x2 = CLAMP(x2, 0, image_width - 1);
    y2 = CLAMP(y2, 0, image_height - 1);

    int width = x2 - x1;
    int height = y2 - y1;

    if (width <= 0 || height <= 0)
    {
        printf("[FAIL] Coordinates are outside of source pixbuf\n");
        return NULL;
    }

    return gdk_pixbuf_new_subpixbuf(pixbuf, x1, y1, width, height);
}
