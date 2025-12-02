#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/image/image.h"
#include "../include/neural_network.h"
#include "../include/line_detection.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>

#define MODEL_PATH "tests/model"

/**
 * Return the length of a string (no strlen allowed).
 */
int len_word(char word[])
{
    int i = 0;
    while (word[i] != '\0')
        i++;
    return i;
}

/**
 * Search for a word inside a 2D grid of characters.
 *
 * rows, cols : grid dimensions
 * tab        : 2D character grid [rows][cols]
 * word       : null-terminated string to search
 * x1,y1      : output coordinates of first letter
 * x2,y2      : output coordinates of last letter
 */
void solve(int rows, int cols, char tab[rows][cols], char word[], int *x1,
           int *y1, int *x2, int *y2)
{
    int len = len_word(word);
    int dir[8][2] = {{1, 1},   {1, 0},  {1, -1}, {0, -1},
                     {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            if (tolower(tab[y][x]) != tolower(word[0]))
            {
                continue;
            }

            for (int d = 0; d < 8; d++)
            {
                int dx = dir[d][0];
                int dy = dir[d][1];

                int cx = x;
                int cy = y;
                int i = 1;

                while (i < len)
                {
                    cx += dx;
                    cy += dy;

                    if (cx < 0 || cx >= cols || cy < 0 || cy >= rows)
                    {
                        break;
                    }

                    if (tolower(tab[cy][cx]) != tolower(word[i]))
                    {

                        break;
                    }

                    i++;
                }
                if (i == len)
                {
                    *x1 = x;
                    *y1 = y;
                    *x2 = cx;
                    *y2 = cy;
                    return;
                }
            }
        }
    }

    *x1 = *y1 = *x2 = *y2 = -1;
}

int center_x(const GridLetter *grid_letter)
{
    return (grid_letter->x1 + grid_letter->x2) / 2;
}

int center_y(const GridLetter *grid_letter)
{
    return (grid_letter->y1 + grid_letter->y2) / 2;
}

int compare_x(const void *letter1, const void *letter2)
{
    const GridLetter *grid_letter1 = letter1;
    const GridLetter *grid_letter2 = letter2;

    return center_x(grid_letter1) - center_x(grid_letter2);
}

int compare_y(const void *letter1, const void *letter2)
{
    const GridLetter *grid_letter1 = letter1;
    const GridLetter *grid_letter2 = letter2;

    return center_y(grid_letter1) - center_y(grid_letter2);
}

GridLetter **build_grid_from_image(GridLetter *letters, int nb_letters,
                                   int *rows_out, int *cols_out)
{
    if (nb_letters <= 0)
    {
        printf("No letters were detected while trying to build the grid array "
               "!\n");
        return NULL;
    }

    // Sort letters by y value
    qsort(letters, nb_letters, sizeof(GridLetter), compare_y);

    int row_threshold = 5; // The tolerance between 2 y letter values to group
                           // them in the same row
    GridLetter **temp_rows = malloc(nb_letters * sizeof(GridLetter *));
    int *row_sizes = calloc(nb_letters, sizeof(int));

    temp_rows[0] = malloc(nb_letters * sizeof(GridLetter));
    temp_rows[0][0] = letters[0];
    row_sizes[0] = 1;
    int row_count = 1; // The number of rows in the grid
    int col_count = 1; // The number of columns in the grid

    for (int i = 1; i < nb_letters; i++)
    {
        int center_y_prev = center_y(&letters[i - 1]);
        int center_y_current = center_y(&letters[i]);

        // Same row
        if (abs(center_y_current - center_y_prev) <= row_threshold)
        {
            temp_rows[row_count - 1][row_sizes[row_count - 1]] = letters[i];
            if (row_sizes[row_count - 1] > col_count)
            {
                col_count = row_sizes[row_count - 1];
            }
            row_sizes[row_count - 1]++;
        }
        // Start new row
        else
        {
            temp_rows[row_count] = malloc(nb_letters * sizeof(GridLetter));
            temp_rows[row_count][0] = letters[i];
            row_sizes[row_count] = 1;
            row_count++;
        }
    }

    // Sort every row by x value
    for (int row = 0; row < row_count; row++)
    {
        qsort(temp_rows[row], sizeof(GridLetter), row_sizes[row], compare_x);
    }

    // Create the sorted grid
    GridLetter **grid = malloc(row_count * sizeof(GridLetter *));
    for (int row = 0; row < row_count; row++)
    {
        grid[row] = malloc(col_count * sizeof(GridLetter));
        memcpy(grid[row], temp_rows[row], col_count * sizeof(GridLetter));
    }

    for (int i = 0; i < row_count; i++)
    {
        free(temp_rows[i]);
    }
    free(temp_rows);
    free(row_sizes);

    *rows_out = row_count;
    *cols_out = col_count;

    return grid;
}

// Builds the array of the grid with the letters found in the grid in the image
char **build_grid_array(GdkPixbuf *pixbuf, GridLetter **grid_letters, int rows,
                        int cols)
{
    NeuralNetwork *nn = load_network(MODEL_PATH);
    if (!nn)
    {
        printf("Neural network could not be loaded, check if a model exists in "
               "tests/model\n");
        return NULL;
    }

    char **grid_array = malloc(rows * sizeof(char *));
    for (int row = 0; row < rows; row++)
    {
        grid_array[row] = malloc(cols * sizeof(char));
    }

    // For each letter, determine the character with the Neural Network and add
    // it to the grid array
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            GridLetter grid_letter = grid_letters[row][col];
            GdkPixbuf *letter = crop(pixbuf, grid_letter.x1, grid_letter.y1,
                                     grid_letter.x2, grid_letter.y2);
            if (!letter)
            {
                printf("No letter found at row : %i, col %i\n", row, col);
                continue;
            }
            GdkPixbuf *scaled_letter = scale_pixbuf_to_28x28(letter);

            char predicted_letter = predict_letter(nn, scaled_letter);
            printf("Letter row : %i, col : %i : %c\n", row, col, predicted_letter);
            grid_array[row][col] = predicted_letter;

            g_object_unref(letter);
            g_object_unref(scaled_letter);
        }
    }

    free_network(nn);
    for (int i = 0; i < rows; i++)
    {
        free(grid_letters[i]);
    }
    free(grid_letters);

    return grid_array;
}

/*
// Return an array of the solved words' coordinates in the grid
int **get_all_words_coordinates(int rows, int cols, char tab[rows][cols],
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

void solve_grid() {}
*/
