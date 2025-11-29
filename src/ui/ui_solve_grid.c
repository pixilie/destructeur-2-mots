#include <gdk-pixbuf/gdk-pixbuf.h>


// Draws a red pixel of size thickness * thickness
void draw_pixel(GdkPixbuf *pixbuf, int x, int y, int thickness)
{ 
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int dy = - thickness / 2; dy <= thickness / 2; dy++)
    {
        for (int dx = - thickness / 2; dx <= thickness / 2; dx++)
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

// Draws a red line from (x1, y1) to (x2, y2) in the UI colored image
// Uses Bresenham's line algorithm to draw the ideal straight line
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int error_term = dx - dy;

    while (1)
    {
        draw_pixel(pixbuf, x1, y1, thickness);
        
        if (x1 == x2 && y1 == y2)
        {
            break; // Reached the end of the line
        }

        int error_term2 = 2 * error_term;

        if (error_term2 > - dy) // Move horizontally
        {
            error_term -= dy;
            x1 += 1;
        }

        if (error_term2 < dx) // Move vertically
        {
            error_term += dx;
            y1 += 1;
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

// Return a 
int[][4] get_all_words_coordinates()

void find_solved_word() {}

void solve_grid() { find_solved_word(); }
