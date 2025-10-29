#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>

/**
 * convert_to_grayscale:
 * Convert an RGB(A) GdkPixbuf to grayscale in-place.
 *
 * Parameters:
 *  - pixbuf: pointer to a writable GdkPixbuf containing RGB or RGBA pixels.
 *
 * Returns:
 *  - void
 *
 * Side effects:
 *  - Modifies the pixbuf pixel data in-place: for each pixel the R, G and B
 *    channels are replaced by the computed luminance value.
 *
 * Details:
 *  - Uses the standard luminance formula: Gray = 0.299*R + 0.587*G + 0.114*B.
 *  - Preserves the alpha channel if present (alpha is not modified).
 *  - Assumes 8-bit channels and that the pixbuf provided is writable.
 */
void convert_to_grayscale(GdkPixbuf *pixbuf)
{
    // Formula for grayscale: Gray = 0.299 × R + 0.587 × G + 0.114 × B

    // Get image dimensions
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    // Get channels per pixel: (R, G, B) = 3, (R, G, B, A) = 4
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    // Get array of pixeks
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Get rowstride = bytes between 2 lines (can have unused bytes at the end
    // of each row)
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    for (int y = 0; y < height; y++)
    {
        guchar *rows = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *pixel = rows + x * n_channels;
            guchar r = pixel[0]; // R
            guchar g = pixel[1]; // G
            guchar b = pixel[2]; // B

            // Calculate gray formula and make every R, G and B the same
            // grayness
            guchar gray = (guchar)(0.299 * r + 0.587 * g + 0.114 * b);
            for (int i = 0; i < 3; i++)
            {
                pixel[i] = gray;
            }
        }
    }
}

/**
 * binarize_image:
 * Binarize a grayscale GdkPixbuf in-place using a threshold.
 *
 * Parameters:
 *  - pixbuf   : pointer to a writable GdkPixbuf expected to be grayscale
 *               (R==G==B for each pixel) or at least RGB/RGBA.
 *  - threshold: integer threshold in [0,255]. Pixels with gray < threshold
 *               become black (0), others become white (255).
 *
 * Returns:
 *  - void
 *
 * Side effects:
 *  - Modifies the pixbuf pixel data in-place, setting R,G,B to 0 or 255.
 *  - Alpha channel, if present, is left unchanged.
 *
 * Notes:
 *  - Typical threshold value is 128. The function does not clamp the
 *    threshold argument; values outside 0..255 behave accordingly due to
 *    comparison with the channel byte.
 */
void binarize_image(GdkPixbuf *pixbuf, int treshold)
{
    // Transform gray image into a black and white image
    // For each pixel in image:
    //  - if gray < treshold -> black pixel
    //  - if gray >= treshold -> white pixel
    // Treshold is 128 by default

    // Get image dimensions
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    // Get channels per pixel: (R, G, B) = 3, (R, G, B, A) = 4
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    // Get array of pixeks
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Get rowstride = bytes between 2 lines (can have unused bytes at the end
    // of each row)
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    for (int y = 0; y < height; y++)
    {
        guchar *rows = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *pixel = rows + x * n_channels;
            guchar gray = pixel[0];
            if (gray < treshold)
            {
                gray = 0; // Black pixel
            }
            else
            {
                gray = 255; // White pixel
            }
            for (int i = 0; i < 3; i++)
            {
                pixel[i] = gray;
            }
        }
    }
}
