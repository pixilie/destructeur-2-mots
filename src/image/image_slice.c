#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <stdlib.h>

GdkPixbuf **slice_from(GdkPixbuf *pixbuf, int x, int y, int direction)
{
    if (!pixbuf)
        return NULL;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    GdkPixbuf **result = malloc(2 * sizeof(GdkPixbuf*));
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
        result[0] = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, width, y);       // top
        result[1] = gdk_pixbuf_new_subpixbuf(pixbuf, 0, y, width, new_height); // bottom
    }
    else // vertical cut
    {
        if (x <= 0 || x >= width)
        {
            free(result);
            return NULL;
        }

        int new_width = width - x;
        result[0] = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, x, height);      // left
        result[1] = gdk_pixbuf_new_subpixbuf(pixbuf, x, 0, new_width, height); // right
    }

    return result;
}

GdkPixbuf **slice_in_n(GdkPixbuf *pixbuf, int n_slice)
{
    if (!pixbuf || n_slice <= 0)
        return NULL;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    if (width != height)
        return NULL;

    int size_each = width / n_slice;
    int total = n_slice * n_slice;

    GdkPixbuf **tiles = malloc(total * sizeof(GdkPixbuf*));
    if (!tiles)
        return NULL;

    int index = 0;
    for (int i = 0; i < n_slice; i++)
    {
        for (int j = 0; j < n_slice; j++)
        {
            int x = j * size_each;
            int y = i * size_each;

            tiles[index++] = gdk_pixbuf_new_subpixbuf(pixbuf, x, y, size_each, size_each);
        }
    }

    return tiles;
}
