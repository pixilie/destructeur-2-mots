#include "../../include/grid_cutting/invert_color.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

/**
 * invert_color:
 * Invert the grayscale color values of a pixbuf in-place (black↔white).
 *
 * Parameters:
 *  - pixbuf: pointer to a GdkPixbuf expected to be grayscale (R==G==B).
 */
void invert_color(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            guchar *p = pixels + y * rowstride + x * n_channels;
            p[0] = p[1] = p[2] = 255 - p[0];
        }
    }
}
