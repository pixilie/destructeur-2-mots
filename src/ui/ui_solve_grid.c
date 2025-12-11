#include "../../include/solver.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

// Draws a pixel of size thickness * thickness with custom GDB color
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

// Draws a red line from (x1, y1) to (x2, y2) in the UI colored image
// Uses Bresenham's line algorithm to draw the ideal straight line
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness, int color[3])
{    
    if (x1 == x2 && y1 == y2)
    {
        draw_pixel(pixbuf, x1, y1, thickness, color);
        return;
    }
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int step_x = (x1 < x2) ? 1 : -1; // The direction of the horizontal movement
    int step_y = (y1 < y2) ? 1 : -1;
        
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

// Draws a red rectangle connecting 4 points on the UI image, marking a word as
// resolved
// Can mark horizontal, vertical and diagonal words as resolved
// (x1, y1) : Top left corner
// (x2, y2) : Top right corner
// (x3, y3) : Bottom right corner
// (x4, y4) : Bottom left corner
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int x3,
                    int y3, int x4, int y4, int thickness)
{
    // Draw color based on orintation of word
    int color[3];
    int rectangle_width = abs(x2 - x1);
    int rectangle_height = abs(y3 - y2);
    if (y1 == y2 && y3 == y4 && rectangle_height < rectangle_width)
    {
        color[0] = 255; // Horizontal words : red rectangle
    }
    else if (y1 == y2 && y3 == y4 && rectangle_width < rectangle_height)
    {
        color[1] = 255; // Vertical words : Green rectangle
    }
    else
    {
        color[2] = 255;
    }
       
    draw_line(pixbuf, x1, y1, x2, y2, thickness, color);
    draw_line(pixbuf, x2, y2, x3, y3, thickness, color);
    draw_line(pixbuf, x3, y3, x4, y4, thickness, color);
    draw_line(pixbuf, x4, y4, x1, y1, thickness, color);
}

// Return an array of the solved words' coordinates in the grid
int **get_all_words_coordinates(int rows, int cols, char** tab,
                                int words_count, char **words)
{
    int **result = malloc(words_count * sizeof(int *));
    for (int word_index = 0; word_index < words_count; word_index++)
    {
        result[word_index] = malloc(4 * sizeof(int));

        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;
        char *word = words[word_index];
        solve(rows, cols, tab, word, &x1, &y1, &x2, &y2);
        result[word_index][0] = x1;
        result[word_index][1] = y1;
        result[word_index][2] = x2;
        result[word_index][3] = y2;
    }
    return result;
}

// Takes a solved word coordinates in the grid and returns its coordinates in
// the image
int *get_word_image_coordinates(int grid_coos[4], int rows, int cols, int x1,
                                int y1, int x2, int y2)
{
    int grid_width = grid_coos[2] - grid_coos[0];
    int grid_height = grid_coos[3] - grid_coos[1];

    int width_per_letter = grid_width / cols;
    int height_per_letter = grid_height / rows;

    int *result = calloc(8, sizeof(int));

    result[0] = grid_coos[0] + x1 * width_per_letter;
    result[1] = grid_coos[0] + x1 * height_per_letter;

    result[2] = grid_coos[0] + y1 * width_per_letter;
    result[3] = grid_coos[0] + y1 * height_per_letter;

    result[4] = grid_coos[0] + x2 * width_per_letter;
    result[5] = grid_coos[0] + x2 * height_per_letter;

    result[6] = grid_coos[0] + y2 * width_per_letter;
    result[7] = grid_coos[0] + y2 * height_per_letter;

    return result;
}
