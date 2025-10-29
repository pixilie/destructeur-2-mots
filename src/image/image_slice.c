#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <stdlib.h>

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

GdkPixbuf **slice_in_n(GdkPixbuf *pixbuf, int n_slice) // FIX
{
    if (!pixbuf || n_slice <= 0)
        return NULL;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    //if (width != height)
    //    return NULL;

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

GdkPixbuf *crop(GdkPixbuf *src, int x1, int y1, int x2, int y2)
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

    int width = x2 - x1;
    int height = y2 - y1;

    int src_width = gdk_pixbuf_get_width(src);
    int src_height = gdk_pixbuf_get_height(src);

    if (x1 < 0 || y1 < 0 || x2 > src_width || y2 > src_height)
    {
        printf("[FAIL] Coordinates are outside of source pixbuf\n");
        return NULL;
    }

    return gdk_pixbuf_new_subpixbuf(src, x1, y1, width, height);
}
