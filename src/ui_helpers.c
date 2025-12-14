#include <gdk-pixbuf/gdk-pixbuf.h>

/**
 * draw_pixel:
 * Draws a colored pixel on the (x, y) coordinates in the pixbuf with a size of
 * thickness.
 *
 * Parameters:
 *  - pixbuf    : pointer to a RGB GdkPixbuf.
 *  - x         : the x coordinate of the pixel.
 *  - y         : the y coordinate of the pixel.
 *  - thickness : the size of the pixel.
 *  - color     : int array of [R, G, B] containing the RGB color of the pixel
 * to draw.
 */
void draw_pixel(GdkPixbuf *pixbuf, int x, int y, int thickness, int color[3])
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
            pixel[0] = color[0];
            pixel[1] = color[1];
            pixel[2] = color[2];
        }
    }
}

/**
 * draw_line:
 * Draws a colored line from (x1, y1) to (x2, y2) in the pixbuf with a size of
 * thickness. Uses Bresenham's line algorithm to draw the ideal straight line.
 *
 * Parameters:
 *  - pixbuf    : pointer to a RGB GdkPixbuf.
 *  - x1        : the starting x coordinate of the line to draw.
 *  - y1        : the starting y coordinate of the line to draw.
 *  - x2        : the ending x coordinate of the line to draw.
 *  - y2        : the ending y coordinate of the line to draw.
 *  - thickness : the size of the line.
 *  - color     : int array of [R, G, B] containing the RGB color of the line to
 * draw.
 */
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness,
               int color[3])
{
    if (x1 == x2 && y1 == y2)
    {
        draw_pixel(pixbuf, x1, y1, thickness, color);
        return;
    }

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int step_x = (x1 < x2) ? 1 : -1; // The direction of the horizontal movement
    int step_y = (y1 < y2) ? 1 : -1; // The direction of the vertical movement

    int error_term = dx - dy;

    while (1)
    {
        draw_pixel(pixbuf, x1, y1, thickness, color);

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

/**
 * draw_rectangle:
 * Draws a colored rectangle from 4 points in the pixbuf with a size of
 * thickness.
 *
 * Parameters:
 *  - pixbuf    : pointer to a RGB GdkPixbuf.
 *  - (x1, y1)        : top left corner coordinates.
 *  - (x2, y2)        : top right corner coordinates.
 *  - (x3, y3)        : bottom right corner coordinates.
 *  - (x4, y4)        : bottom left corner coordinates.
 *  - thickness : the size of the rectangle lines.
 */
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int x3,
                    int y3, int x4, int y4, int thickness)
{
    // Draw color based on orintation of word
    int color[3] = {0, 0, 0};
    int rectangle_width = abs(x3 - x1);
    int rectangle_height = abs(y3 - y1);

    int min_width = 100;
    int min_height = 100;
    if (rectangle_width > min_width && rectangle_height < min_height)
    {
        color[0] = 255; // Horizontal words : Red rectangle
    }
    else if (rectangle_width < min_width && rectangle_height > min_height)
    {
        color[1] = 200; // Vertical words : Green rectangle
    }
    else
    {
        color[2] = 255; // Diagonal words : Blue rectangle
    }

    draw_line(pixbuf, x1, y1, x2, y2, thickness, color);
    draw_line(pixbuf, x2, y2, x3, y3, thickness, color);
    draw_line(pixbuf, x3, y3, x4, y4, thickness, color);
    draw_line(pixbuf, x4, y4, x1, y1, thickness, color);
}
