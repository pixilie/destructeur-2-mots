#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
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

int calculate_mean_treshold(GdkPixbuf *pixbuf)
{ 
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    long sum = 0; //sum of all gray pixels
    int total_pixel_count = width * height;
    
    for (int y = 0; y < height; y++)
    {
        guchar *rows = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *pixel = rows + x * n_channels;
            sum += pixel[0]; //Add pixel value to sum of all gray pixels
        }
    }

    return (int)(sum / total_pixel_count);
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
 */
void binarize_image(GdkPixbuf *pixbuf, int threshold)
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
            if (gray < threshold)
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

int convert_to_black_and_white(GdkPixbuf *pixbuf)
{
    int threshold = calculate_mean_treshold(pixbuf);
    binarize_image(pixbuf, threshold);
    return threshold;
}

int* create_histogram(GdkPixbuf *pixbuf)
{
    int* histogram = calloc(256, sizeof(int));

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
            histogram[pixel[0]]++;
        }
    }

    return histogram;
}

void print_histogram(GdkPixbuf *pixbuf)
{
    int *histogram = create_histogram(pixbuf);

    printf("Histogram of pixels:\n");
    
    for (int i = 0; i < 256; i++) //Print histogram[0] to histogram[255] of values
    {
        printf("Pixel %i : %i pixels\n", i, histogram[i]);
    }

    free(histogram);
}
