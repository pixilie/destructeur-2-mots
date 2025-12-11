#include "../../include/solver.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

/*
 * draw_pixel:
 * Draw a red square of size thickness x thickness centered at (x, y) in the pixbuf.
 * Parameters:
 *  - pixbuf: target GdkPixbuf (modified in-place).
 *  - x, y: center coordinates of the pixel to draw.
 *  - thickness: size of the square in pixels (odd values center neatly).
 */
void draw_pixel(GdkPixbuf *pixbuf, int x, int y, int thickness)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int dy = -thickness / 2; dy <= thickness / 2; dy++)
    {
        for (int dx = -thickness / 2; dx <= thickness / 2; dx++)
        {
            int new_x = x + dx;
            int new_y = y + dy;
            if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height)
            {
                continue;
            }

            guchar *pixel = pixels + new_y * rowstride + new_x * n_channels;
            pixel[0] = 255; // Red
            pixel[1] = 0;
            pixel[2] = 0;
        }
    }
}

/*
 * draw_line:
 * Draw a red line from (x1, y1) to (x2, y2) into the pixbuf using Bresenham's algorithm.
 * Parameters:
 *  - pixbuf: target GdkPixbuf (modified in-place).
 *  - x1,y1: start coordinates.
 *  - x2,y2: end coordinates.
 *  - thickness: line thickness in pixels.
 */
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness)
{
    if (x1 == x2 && y1 == y2)
    {
        draw_pixel(pixbuf, x1, y1, thickness);
        return;
    }

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int step_x = (x1 < x2) ? 1 : -1; // The direction of the horizontal movement
    int step_y = (y1 < y2) ? 1 : -1;

    int error_term = dx - dy;

    while (1)
    {
        draw_pixel(pixbuf, x1, y1, thickness);

        if (x1 == x2 && y1 == y2)
        {
            break; // Reached the end of the line
        }

        int error_term2 = 2 * error_term;

        if (error_term2 > -dy) // Move horizontally
        {
            error_term -= dy;
            x1 += step_x;
        }

        if (error_term2 < dx) // Move vertically
        {
            error_term += dx;
            y1 += step_y;
        }
    }
}

/*
 * draw_rectangle:
 * Draw a red closed polygon connecting four points on the pixbuf. Used to mark a solved word.
 * Supports horizontal, vertical and diagonal orientations.
 *
 * Parameters:
 *  - pixbuf: target GdkPixbuf (modified in-place).
 *  - (x1,y1): first corner (top-left for standard orientation).
 *  - (x2,y2): second corner (top-right).
 *  - (x3,y3): third corner (bottom-right).
 *  - (x4,y4): fourth corner (bottom-left).
 *  - thickness: line thickness in pixels.
 */
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int x3,
                    int y3, int x4, int y4, int thickness)
{
    draw_line(pixbuf, x1, y1, x2, y2, thickness);
    draw_line(pixbuf, x2, y2, x3, y3, thickness);
    draw_line(pixbuf, x3, y3, x4, y4, thickness);
    draw_line(pixbuf, x4, y4, x1, y1, thickness);
}
