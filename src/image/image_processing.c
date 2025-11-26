#include <gdk-pixbuf/gdk-pixbuf.h>

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
    // Get image dimensions
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    // Get channels per pixel: (R, G, B) = 3, (R, G, B, A) = 4
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    // Get rowstride = bytes between 2 lines (can have unused bytes at the end
    // of each row)
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    // Get array of pixeks
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

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

            // Formula for grayscale: Gray = 0.299 × R + 0.587 × G + 0.114 × B
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

    long sum = 0; // sum of all gray pixels
    int total_pixel_count = width * height;

    for (int y = 0; y < height; y++)
    {
        guchar *rows = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *pixel = rows + x * n_channels;
            sum += pixel[0]; // Add pixel value to sum of all gray pixels
        }
    }

    return (int)(sum / total_pixel_count);
}

int *create_histogram(GdkPixbuf *pixbuf)
{
    int *histogram = calloc(256, sizeof(int));

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

    for (int i = 0; i < 256; i++)
    // Print histogram[0] to histogram[255] of pixel intensities with number of
    // pixels of intensity i
    {
        printf("Intensity %i : %i pixels\n", i, histogram[i]);
    }

    free(histogram);
}

int calculate_otsu_threshold(GdkPixbuf *pixbuf)
{
    int *histogram = create_histogram(pixbuf);

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    int total_pixels = width * height;

    double sum = 0; // Total sum of intensities
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
        {
            continue;
        }

        weight_light = total_pixels - weight_dark;
        if (weight_light == 0)
        {
            break;
        }

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
    //  - if gray < threshold -> black pixel
    //  - if gray >= threshold -> white pixel
    // Threshold is 128 by default

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
    int mean_threshold = calculate_mean_treshold(pixbuf);
    int otsu_threshold = calculate_otsu_threshold(pixbuf);
    int threshold;

    int *histogram = create_histogram(pixbuf);

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    // Calculate mean of intensity distribution
    double mean = 0;
    int total_pixels = width * height;

    for (int i = 0; i < 256; i++)
    {
        mean += histogram[i];
    }
    mean /= total_pixels;

    // Calculate variance of intensity distribution
    double variance = 0;

    for (int i = 0; i < 256; i++)
    {
        variance += (mean - i) * (mean - i) * histogram[i];
    }
    variance /= total_pixels;

    free(histogram);

    // High variance -> Use Otsu threshold
    if (variance > 1500)
    {
        threshold = otsu_threshold;
        // printf("Threshold method chosen : Otsu\n");
    }
    // Low variance -> Use mean threshold
    else
    {
        threshold = mean_threshold;
        // printf("Threshold method chosen : Mean\n");
    }

    binarize_image(pixbuf, threshold);
    return threshold;
}

// Returns the index of the minimum element of the 3x3 neighboring pixels array
int find_minimum_index(int start_index, guchar *neighborhood, int count)
{
    int min_index = start_index;
    for (int i = start_index + 1; i < count; i++)
    {
        if (neighborhood[i] < neighborhood[min_index])
        {
            min_index = i;
        }
    }
    return min_index;
}

// Simple selection sort to sort the array of the 3x3 neighboring pixels to
// calculate the median of the array
void selection_sort(guchar *neighborhood, int count)
{
    for (int i = 0; i < count; i++)
    {
        int min_index = find_minimum_index(i, neighborhood, count);
        int temp = neighborhood[i];
        neighborhood[i] = neighborhood[min_index];
        neighborhood[min_index] = temp;
    }
}

// Filter a 3x3 neighborhood
void filter_neighborhood_3x3(GdkPixbuf *pixbuf, guchar *copy, int x, int y,
                             int width, int height, int rowstride,
                             int n_channels)
{
    guchar neighborhood[9]; // The 9 neighboring pixels array
    int neighborhood_count = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int new_x = x + dx;
            int new_y = y + dy;

            // Skip out of bounds neighboring pixels
            if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height)
            {
                continue;
            }

            guchar *p = copy + new_y * rowstride + new_x * n_channels;

            neighborhood[neighborhood_count] = p[0];
            neighborhood_count++;
        }
    }

    selection_sort(neighborhood, neighborhood_count);

    guchar median; // The median of the 3x3 pixels

    // Even neighborhood count : Median index = average between the 2 middle
    // elements
    if (neighborhood_count % 2 == 0)
    {
        median = (neighborhood[neighborhood_count / 2 - 1] +
                  neighborhood[neighborhood_count / 2]) /
                 2;
    }

    // Uneven neighborhood count : Median index = middle element
    else
    {
        median = neighborhood[neighborhood_count / 2];
    }

    // Replace original pixel with median pixel
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    guchar *dst = (pixels + y * rowstride) + x * n_channels;
    dst[0] = dst[1] = dst[2] = median;
}

void median_filter_3x3(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Copy pixbuf to get the raw pixels before applying the median filter
    guchar *copy = g_malloc(rowstride * height);
    memcpy(copy, pixels, rowstride * height);

    // Filter every pixel in a 3x3 neighborhood
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            filter_neighborhood_3x3(pixbuf, copy, x, y, width, height,
                                    rowstride, n_channels);
        }
    }

    g_free(copy);
}
