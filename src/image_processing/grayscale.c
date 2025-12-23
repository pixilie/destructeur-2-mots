#include "../../include/image_processing/grayscale.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

/*
 * convert_to_grayscale:
 * Convert an RGB(A) GdkPixbuf to grayscale in-place.
 *
 * Parameters:
 *  - pixbuf: writable GdkPixbuf containing RGB or RGBA pixels.
 */
void convert_to_grayscale(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int y = 0; y < height; y++)
    {
        guchar *rows = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *pixel = rows + x * n_channels;
            guchar r = pixel[0];
            guchar g = pixel[1];
            guchar b = pixel[2];

            /* Gray = 0.299*R + 0.587*G + 0.114*B */
            guchar gray = (guchar)(0.299 * r + 0.587 * g + 0.114 * b);
            for (int i = 0; i < 3; i++)
            {
                pixel[i] = gray;
            }
        }
    }
}
