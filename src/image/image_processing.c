#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdlib.h>
#include <stdio.h>

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

/*
 * calculate_mean_treshold:
 * Compute the mean intensity of a grayscale pixbuf.
 *
 * Parameters:
 *  - pixbuf: grayscale GdkPixbuf.
 *
 * Returns:
 *  - integer mean intensity in [0,255].
 */
int calculate_mean_treshold(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    long sum = 0;
    int total_pixel_count = width * height;

    for (int y = 0; y < height; y++)
    {
        guchar *rows = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *pixel = rows + x * n_channels;
            sum += pixel[0];
        }
    }

    return (int)(sum / total_pixel_count);
}

/*
 * create_histogram:
 * Build a histogram of grayscale intensities (0..255).
 *
 * Parameters:
 *  - pixbuf: grayscale GdkPixbuf.
 *
 * Returns:
 *  - pointer to a newly allocated int[256] histogram (caller must free).
 */
int *create_histogram(GdkPixbuf *pixbuf)
{
    int *histogram = calloc(256, sizeof(int));
    if (!histogram)
        return NULL;

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

/*
 * calculate_otsu_threshold:
 * Compute Otsu's threshold for a grayscale image.
 *
 * Parameters:
 *  - pixbuf: grayscale GdkPixbuf.
 *
 * Returns:
 *  - threshold value in [0,255].
 */
int calculate_otsu_threshold(GdkPixbuf *pixbuf)
{
    int *histogram = create_histogram(pixbuf);
    if (!histogram)
        return 128;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int total_pixels = width * height;

    double sum = 0;
    for (int i = 0; i < 256; i++)
    {
        sum += i * histogram[i];
    }

    double sum_dark = 0;
    int weight_dark = 0;
    int weight_light = 0;
    double max_var_between_class = 0;
    int threshold = 128;

    for (int t = 0; t < 256; t++)
    {
        weight_dark += histogram[t];
        if (weight_dark == 0)
            continue;

        weight_light = total_pixels - weight_dark;
        if (weight_light == 0)
            break;

        sum_dark += (double)(t * histogram[t]);
        double mean_dark = sum_dark / weight_dark;
        double mean_light = (sum - sum_dark) / weight_light;

        double var_between_class = (double)weight_dark * (double)weight_light *
                                   (mean_dark - mean_light) *
                                   (mean_dark - mean_light);

        if (var_between_class > max_var_between_class)
        {
            max_var_between_class = var_between_class;
            threshold = t;
        }
    }

    free(histogram);
    return threshold;
}

/*
 * binarize_image:
 * Threshold a grayscale image in-place: pixels < threshold -> 0, else 255.
 *
 * Parameters:
 *  - pixbuf: grayscale or RGB(A) GdkPixbuf.
 *  - threshold: integer threshold in [0,255].
 */
void binarize_image(GdkPixbuf *pixbuf, int threshold)
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
            guchar gray = pixel[0];
            if (gray < threshold)
                gray = 0;
            else
                gray = 255;

            for (int i = 0; i < 3; i++)
                pixel[i] = gray;
        }
    }
}

/*
 * convert_to_black_and_white:
 * Choose a threshold (mean or Otsu) and binarize the image.
 *
 * Parameters:
 *  - pixbuf: grayscale GdkPixbuf (will be modified).
 *
 * Returns:
 *  - the threshold used.
 */
int convert_to_black_and_white(GdkPixbuf *pixbuf)
{
    int mean_threshold = calculate_mean_treshold(pixbuf);
    int otsu_threshold = calculate_otsu_threshold(pixbuf);
    int threshold;

    int *histogram = create_histogram(pixbuf);
    if (!histogram)
        return mean_threshold;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    /* Compute variance of intensity distribution to choose method */
    double mean = 0;
    int total_pixels = width * height;

    for (int i = 0; i < 256; i++)
    {
        mean += histogram[i];
    }
    mean /= total_pixels;

    double variance = 0;
    for (int i = 0; i < 256; i++)
    {
        variance += (mean - i) * (mean - i) * histogram[i];
    }
    variance /= total_pixels;

    free(histogram);

    if (variance > 1500)
        threshold = otsu_threshold;
    else
        threshold = mean_threshold;

    binarize_image(pixbuf, threshold);
    return threshold;
}