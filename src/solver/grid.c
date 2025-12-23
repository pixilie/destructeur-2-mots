#include "../../include/image_processing/image_processing.h"
#include "../../include/neural_network.h"
#include "../../include/solver/compare_letter.h"
#include "../../include/solver/letter.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

/*
 * build_grid_from_image:
 * Organize detected Letter bounding boxes into a 2D grid (rows x cols).
 * Returns a newly allocated Letter** grid and writes rows/cols to outputs.
 */
Letter **build_grid_from_image(Letter *grid_letters, int nb_letters,
                               int *rows_out, int *cols_out)
{
    if (nb_letters <= 0)
    {
        printf("No letters were detected while trying to build the grid array "
               "!\n");
        return NULL;
    }

    // Sort letters by y value
    qsort(grid_letters, nb_letters, sizeof(Letter), compare_y);

    int row_threshold = 30; // The tolerance between 2 y letter values to group
                            // them in the same row
    Letter **temp_rows = calloc(nb_letters, sizeof(Letter *));
    int *row_sizes = calloc(nb_letters, sizeof(int));

    temp_rows[0] = calloc(nb_letters, sizeof(Letter));
    temp_rows[0][0] = grid_letters[0];
    row_sizes[0] = 1;
    int row_count = 1; // The number of rows in the grid
    int col_count = 1; // The number of columns in the grid

    int first_letter_in_row = 0;
    for (int i = 1; i < nb_letters; i++)
    {
        int center_y_prev = center_y(&grid_letters[first_letter_in_row]);
        int center_y_current = center_y(&grid_letters[i]);

        // Same row
        if (abs(center_y_current - center_y_prev) <= row_threshold)
        {
            temp_rows[row_count - 1][row_sizes[row_count - 1]] =
                grid_letters[i];
            row_sizes[row_count - 1]++;
        }
        // Start new row
        else
        {
            row_count++;
            first_letter_in_row = i;
            temp_rows[row_count - 1] = calloc(nb_letters, sizeof(Letter));
            temp_rows[row_count - 1][0] = grid_letters[i];
            row_sizes[row_count - 1] = 1;
        }
    }

    // Number of columns = Row with the maximum number of letters
    for (int row = 0; row < row_count; row++)
    {
        if (row_sizes[row] > col_count)
        {
            col_count = row_sizes[row];
        }
    }

    // Sort every row by x value
    for (int row = 0; row < row_count; row++)
    {
        qsort(temp_rows[row], row_sizes[row], sizeof(Letter), compare_x);
    }

    // Create the sorted grid
    Letter **grid = calloc(row_count, sizeof(Letter *));
    for (int row = 0; row < row_count; row++)
    {
        grid[row] = calloc(col_count, sizeof(Letter));
        memcpy(grid[row], temp_rows[row], row_sizes[row] * sizeof(Letter));

        for (int col = row_sizes[row]; col < col_count; col++)
        {
            grid[row][col].x1 = 0;
            grid[row][col].y1 = 0;
            grid[row][col].x2 = 0;
            grid[row][col].y2 = 0;
        }
    }

    for (int i = 0; i < row_count; i++)
    {
        free(temp_rows[i]);
    }
    free(temp_rows);
    free(row_sizes);
    free(grid_letters);

    *rows_out = row_count;
    *cols_out = col_count;

    return grid;
}

/*
 * build_grid_array:
 * Recognize each Letter region from pixbuf using the neural network and build
 * a rows x cols char grid (A-Z). Returns malloc'd char** and writes rows/cols.
 */
char **build_grid_array(NeuralNetwork *nn, GdkPixbuf *pixbuf,
                        Letter **grid_letters, int rows, int cols,
                        int *rows_out, int *cols_out)
{
    if (!nn)
    {
        printf(
            "Neural network could not be loaded, check if the model exists\n");
        return NULL;
    }

    char **grid_array = calloc(rows, sizeof(char *));
    for (int row = 0; row < rows; row++)
    {
        grid_array[row] = calloc(cols, sizeof(char));
    }

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int os = 3;

    // For each letter, determine the character with the Neural Network and add
    // it to the grid array
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            Letter grid_letter = grid_letters[row][col];
            if (grid_letter.x1 <= 0 || grid_letter.y1 <= 0 ||
                grid_letter.x2 >= width || grid_letter.y2 >= height)
            {
                continue;
            }
            GdkPixbuf *letter =
                crop(pixbuf, grid_letter.x1 - os, grid_letter.y1 - os,
                     grid_letter.x2 + os, grid_letter.y2 + os);

            median_filter_3x3(letter);

            if (!letter)
            {
                continue;
            }

            char predicted_letter = predict_letter(nn, letter);
            grid_array[row][col] = predicted_letter;

            g_object_unref(letter);
        }
    }

    int start_row = -1;
    int end_row = -1;
    int col_count = 0;

    // Find first and last rows that are not empty
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            if (grid_array[row][col] >= 'A' && grid_array[row][col] <= 'Z')
            {
                if (start_row == -1)
                {
                    start_row = row;
                }
                end_row = row;
            }
        }
    }

    // Find number of cols that are not empty
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            if (grid_array[row][col] >= 'A' && grid_array[row][col] <= 'Z')
            {
                if (col + 1 > col_count)
                {
                    col_count = col + 1;
                }
            }
        }
    }

    // Remove last row if row with very few letters (trash rows)
    int min_letters_per_row = 5;
    int last_row_count = 0;
    for (int col = 0; col < col_count; col++)
    {
        if (grid_array[end_row][col] >= 'A' && grid_array[end_row][col] <= 'Z')
        {
            last_row_count++;
        }
    }

    if (last_row_count < min_letters_per_row)
    {
        end_row--; // Remove trash empty last row
    }

    int row_count = end_row - start_row + 1;

    char **new_grid_array = calloc(row_count, sizeof(char *));
    for (int row = 0; row < row_count; row++)
    {
        new_grid_array[row] = calloc(col_count, sizeof(char));
        for (int col = 0; col < col_count; col++)
        {
            new_grid_array[row][col] = grid_array[start_row + row][col];
        }
    }

    *rows_out = row_count;
    *cols_out = col_count;

    for (int row = 0; row < rows; row++)
    {
        free(grid_array[row]);
    }
    free(grid_array);

    printf(COLOR_YELLOW "[INFO] " COLOR_RESET
                        "Grid detected: %i rows, %i columns\n",
           row_count, col_count);
    for (int row = 0; row < row_count; row++)
    {
        printf("[");
        for (int col = 0; col < col_count; col++)
        {
            printf("%c", new_grid_array[row][col]);
            if (col < col_count - 1)
            {
                printf(" ");
            }
        }
        printf("]\n");
    }

    return new_grid_array;
}
