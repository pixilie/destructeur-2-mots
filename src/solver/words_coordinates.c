#include "../../include/image_processing/image_processing.h"
#include "../../include/neural_network.h"
#include "../../include/solver/solver.h"

#include <math.h>

/*
 * get_solved_words_grid_coos:
 * For each solved word string, find its start/end coordinates in the grid
 * using solve() and return an array of [x1,y1,x2,y2] entries.
 */
int **get_solved_words_grid_coos(char **words, int words_count, char **grid,
                                 int rows, int cols)
{
    if (!words)
    {
        printf("No solved words found !\n");
        return NULL;
    }

    int **words_coos = calloc(words_count, sizeof(int *));
    for (int word_index = 0; word_index < words_count; word_index++)
    {
        words_coos[word_index] = calloc(4, sizeof(int));
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;

        char *word = words[word_index];
        solve(rows, cols, grid, word, &x1, &y1, &x2, &y2);
        words_coos[word_index][0] = x1;
        words_coos[word_index][1] = y1;
        words_coos[word_index][2] = x2;
        words_coos[word_index][3] = y2;
    }

    return words_coos;
}

/*
 * get_solved_words_image_coos_drawing:
 * Convert solved word grid coordinates into image-space polygon coords for
 * drawing (returns array of 8 ints per word: 4 corner points).
 */
int **get_solved_words_image_coos_drawing(int **words_grid_coos,
                                          int words_count, int grid_coos[4],
                                          int rows, int cols)
{
    if (!words_grid_coos)
    {
        printf("No solved words grid coordinates found !\n");
        return NULL;
    }

    int **words_coos_image = calloc(words_count, sizeof(int *));

    int grid_width = grid_coos[2] - grid_coos[0];
    int grid_height = grid_coos[3] - grid_coos[1];

    float width_per_letter = (float)grid_width / cols;
    float height_per_letter = (float)grid_height / rows;

    int start_x = grid_coos[0];
    int start_y = grid_coos[1];

    float extension = width_per_letter / 2.0f;
    float rectangle_skrink = 2.0f;

    for (int w = 0; w < words_count; w++)
    {
        words_coos_image[w] = calloc(8, sizeof(int));

        int x1_grid = words_grid_coos[w][0];
        int y1_grid = words_grid_coos[w][1];
        int x2_grid = words_grid_coos[w][2];
        int y2_grid = words_grid_coos[w][3];

        // Word not found
        if (x1_grid == -1 || y1_grid == -1 || x2_grid == -1 || y2_grid == -1)
        {
            for (int i = 0; i < 8; i++)
            {
                words_coos_image[w][i] = -1;
            }
            continue;
        }

        // Start and end in pixel coordinates (center of letters)
        float x1 =
            start_x + x1_grid * width_per_letter + width_per_letter / 2.0f;
        float y1 =
            start_y + y1_grid * height_per_letter + height_per_letter / 2.0f;
        float x2 =
            start_x + x2_grid * width_per_letter + width_per_letter / 2.0f;
        float y2 =
            start_y + y2_grid * height_per_letter + height_per_letter / 2.0f;

        float dx = x2 - x1;
        float dy = y2 - y1;
        float length = sqrtf(dx * dx + dy * dy);

        float ux = dx / length;
        float uy = dy / length;

        // Extend start and end points along the word direction
        x1 -= ux * extension;
        y1 -= uy * extension;
        x2 += ux * extension;
        y2 += uy * extension;

        float px = -uy;
        float py = ux;

        // Calculate thickness based on orientation
        float thickness;
        if (fabs(dx) < 0.01f) // Vertical word
        {
            thickness = width_per_letter / 2.0f;
        }
        else if (fabs(dy) < 0.01f) // Horizontal word
        {
            thickness = height_per_letter / 2.0f;
        }
        else // Diagonal word
        {
            thickness = (width_per_letter + height_per_letter) / 4.0f;
        }

        thickness -= rectangle_skrink;

        // Four corners of rotated rectangle
        words_coos_image[w][0] = (int)(x1 + px * thickness - 0.5f);
        words_coos_image[w][1] = (int)(y1 + py * thickness - 0.5f);

        words_coos_image[w][2] = (int)(x2 + px * thickness - 0.5f);
        words_coos_image[w][3] = (int)(y2 + py * thickness - 0.5f);

        words_coos_image[w][4] = (int)(x2 - px * thickness - 0.5f);
        words_coos_image[w][5] = (int)(y2 - py * thickness - 0.5f);

        words_coos_image[w][6] = (int)(x1 - px * thickness - 0.5f);
        words_coos_image[w][7] = (int)(y1 - py * thickness - 0.5f);
    }

    return words_coos_image;
}
