#include <gdk-pixbuf/gdk-pixbuf.h>

// Draws a red line from (x1, y1) to (x2, y2) in the UI colored image
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    // Make coordinates in image bounds
    if (x1 < 0)
        x1 = 0;
    if (x2 < 0)
        x2 = 0;
    if (y1 < 0)
        y1 = 0;
    if (y2 < 0)
        y2 = 0;
    if (x1 >= width)
        x1 = width - 1;
    if (x2 >= width)
        x2 = width - 1;
    if (y1 >= height)
        y1 = height - 1;
    if (y2 >= height)
        y2 = height - 1;

    // Horizontal line
    if (y1 == y2)
    {
        for (int y = y1 - thickness / 2; y <= y1 + thickness / 2; y++)
        {
            // Out of bounds -> Skip line
            if (y < 0 || y >= height)
            {
                continue;
            }

            guchar *row = pixels + y * rowstride;

            for (int x = x1; x <= x2; x++)
            {
                guchar *pixel = row + x * n_channels;
                pixel[0] = 255; // Red
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
    }

    // Vertical line
    if (x1 == x2)
    {
        for (int x = x1 - thickness / 2; x <= x1 + thickness / 2; x++)
        {
            // Out of bounds -> Skip column
            if (x < 0 || x >= width)
            {
                continue;
            }

            for (int y = y1; y <= y2; y++)
            {
                guchar *row = pixels + y * rowstride;
                guchar *pixel = row + x * n_channels;
                pixel[0] = 255; // Red
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
    }
}

// Draws a red rectangle on the UI image, marking a word as resolved
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2,
                    int thickness)
{
    draw_line(pixbuf, x1, y1, x2, y1, thickness); // Top
    draw_line(pixbuf, x1, y2, x2, y2, thickness); // Bottom
    draw_line(pixbuf, x1, y1, x1, y2, thickness); // Left
    draw_line(pixbuf, x2, y1, x2, y2, thickness); // Right
}

void find_solved_word() {}

void solve_grid() { find_solved_word(); }
