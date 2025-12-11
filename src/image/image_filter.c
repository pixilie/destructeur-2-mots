#include <gdk-pixbuf/gdk-pixbuf.h>

void erode_3x3(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    guchar *copy = g_malloc(rowstride * height);
    memcpy(copy, pixels, rowstride * height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int black_neighbor = 0;

            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                    {
                        continue;
                    }

                    guchar *p = copy + ny * rowstride + nx * n_channels;

                    if (p[0] == 0) // Black pixel
                    {
                        black_neighbor++;
                    }
                }
            }

            guchar *dst = pixels + y * rowstride + x * n_channels;

            if (black_neighbor == 0)
            {
                dst[0] = dst[1] = dst[2] =
                    255; // Turn isolated black pixels to white
            }
        }
    }
    g_free(copy);
}

void dilate_3x3(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    guchar *copy = g_malloc(rowstride * height);
    memcpy(copy, pixels, rowstride * height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int black_neighbor = 0;

            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                    {
                        continue;
                    }

                    guchar *p = copy + ny * rowstride + nx * n_channels;

                    if (p[0] == 0)
                    {
                        black_neighbor++;
                    }
                }
            }

            guchar *dst = pixels + y * rowstride + x * n_channels;

            if (black_neighbor > 0)
                dst[0] = dst[1] = dst[2] = 0; // Restore nearby pixels to black
        }
    }

    g_free(copy);
}

int find_minimum_index(int start_index, guchar *neighborhood)
{
    int min_index = start_index;
    for (int i = start_index + 1; i < 9; i++)
    {
        if (neighborhood[i] < neighborhood[min_index])
        {
            min_index = i;
        }
    }
    return min_index;
}

void selection_sort(guchar *neighborhood)
{
    for (int i = 0; i < 9; i++)
    {
        int min_index = find_minimum_index(i, neighborhood);
        int temp = neighborhood[i];
        neighborhood[i] = neighborhood[min_index];
        neighborhood[min_index] = temp;
    }
}

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

            if (new_x < 0)
            {
                new_x = 0;
            }
            if (new_x >= width)
            {
                new_x = width - 1;
            }
            if (new_y < 0)
            {
                new_y = 0;
            }
            if (new_y >= height)
            {
                new_y = height - 1;
            }

            guchar *p = copy + new_y * rowstride + new_x * n_channels;

            neighborhood[neighborhood_count] = p[0];
            neighborhood_count++;
        }
    }

    selection_sort(neighborhood);

    guchar median = neighborhood[4];

    // Replace original pixel with median pixel
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    guchar *dst = pixels + y * rowstride + x * n_channels;
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
