#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/image/image.h"
#include "../include/line_detection.h"
#include "../include/neural_network.h"
#include "solver.h"
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

int center_x(const Letter *letter) { return (letter->x1 + letter->x2) / 2; }

int center_y(const Letter *letter) { return (letter->y1 + letter->y2) / 2; }

int compare_x(const void *letter_1, const void *letter_2)
{
    const Letter *letter1 = letter_1;
    const Letter *letter2 = letter_2;

    return center_x(letter1) - center_x(letter2);
}

int compare_y(const void *letter_1, const void *letter_2)
{
    const Letter *letter1 = letter_1;
    const Letter *letter2 = letter_2;

    return center_y(letter1) - center_y(letter2);
}

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

    int row_threshold = 12; // The tolerance between 2 y letter values to group
                            // them in the same row
    Letter **temp_rows = malloc(nb_letters * sizeof(Letter *));
    int *row_sizes = calloc(nb_letters, sizeof(int));

    temp_rows[0] = malloc(nb_letters * sizeof(Letter));
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
            temp_rows[row_count - 1][row_sizes[row_count - 1]] = grid_letters[i];
            row_sizes[row_count - 1]++;
        }
        // Start new row
        else
        {
            row_count++;
            first_letter_in_row = i;
            temp_rows[row_count - 1] = malloc(nb_letters * sizeof(Letter));
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
    Letter **grid = malloc(row_count * sizeof(Letter *));
    for (int row = 0; row < row_count; row++)
    {
        grid[row] = malloc(col_count * sizeof(Letter));
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

    *rows_out = row_count;
    *cols_out = col_count;

    return grid;
}

// Builds the array of the grid with the letters found in the grid in the image
char **build_grid_array(GdkPixbuf *pixbuf, Letter **grid_letters, int rows,
                        int cols, int *rows_out, int *cols_out)
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
        grid_array[row] = calloc(cols, sizeof(char));
    }

    int nb_letter = 0;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    // For each letter, determine the character with the Neural Network and add
    // it to the grid array
    for (int row = 0; row < rows; row++)
    {
        // printf("Row %i : Letters %i to %i :\t ", row, row * cols, row * cols
        // + cols);
        for (int col = 0; col < cols; col++)
        {
            Letter grid_letter = grid_letters[row][col];
            if (grid_letter.x1 <= 0 || grid_letter.y1 <= 0 ||
                grid_letter.x2 >= width || grid_letter.y2 >= height)
            {
                // printf("Failed to detect letter %i : Row : %i, col : %i\n",
                // nb_letter, row, col);
                continue;
            }
            GdkPixbuf *letter = crop(pixbuf, grid_letter.x1, grid_letter.y1,
                                     grid_letter.x2, grid_letter.y2);
            if (!letter)
            {
                // printf("No letter found at row : %i, col %i\n", row, col);
                continue;
            }
            GdkPixbuf *scaled_letter = scale_pixbuf_to_28x28(letter);

            char predicted_letter = predict_letter(nn, scaled_letter);
            // printf("%c ", predicted_letter);
            grid_array[row][col] = predicted_letter;

            g_object_unref(letter);
            g_object_unref(scaled_letter);

            nb_letter++;
        }
        // printf("\n");
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
    printf("Built grid with %i rows and %i columns\n", row_count, col_count);

    char **new_grid_array = malloc(row_count * sizeof(char *));
    for (int row = 0; row < row_count; row++)
    {
        printf("Row %i : \t ", row);
        new_grid_array[row] = malloc(col_count * sizeof(char));
        for (int col = 0; col < col_count; col++)
        {
            new_grid_array[row][col] = grid_array[start_row + row][col];
            printf("%c ", new_grid_array[row][col]);
        }
        printf("\n");
    }

    *rows_out = row_count;
    *cols_out = col_count;
    for (int row = 0; row < rows; row++)
    {
        free(grid_array[row]);
    }
    free(grid_array);

    free_network(nn);

    return new_grid_array;
}

Letter **build_words_list_from_image(Letter *words_letters, int nb_letters,
                               int **words_size_out, int *row_count_out)
{
    if (nb_letters <= 0)
    {
        printf("No letters were detected while trying to build the words list "
               "!\n");
        return NULL;
    }

    // Sort letters by y value
    qsort(words_letters, nb_letters, sizeof(Letter), compare_y);

    int row_threshold = 10; // The tolerance between 2 y letter values to group
                            // them in the same row
    Letter **temp_rows = malloc(nb_letters * sizeof(Letter *));
    int *row_sizes = calloc(nb_letters, sizeof(int));


    temp_rows[0] = malloc(nb_letters * sizeof(Letter));
    temp_rows[0][0] = words_letters[0];
    row_sizes[0] = 1;
    int row_count = 1; // The number of rows in the grid
    int col_count = 1; // The number of columns in the grid

    int first_letter_in_row = 0;
    for (int i = 1; i < nb_letters; i++)
    {
        int center_y_prev = center_y(&words_letters[first_letter_in_row]);
        int center_y_current = center_y(&words_letters[i]);

        // Same row
        if (abs(center_y_current - center_y_prev) <= row_threshold)
        {
            temp_rows[row_count - 1][row_sizes[row_count - 1]] = words_letters[i];
            row_sizes[row_count - 1]++;
        }
        // Start new row
        else
        {
            row_count++;
            first_letter_in_row = i;
            temp_rows[row_count - 1] = malloc(nb_letters * sizeof(Letter));
            temp_rows[row_count - 1][0] = words_letters[i];
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
    
    *row_count_out = row_count;

    // Sort every row by x value
    for (int row = 0; row < row_count; row++)
    {
        qsort(temp_rows[row], row_sizes[row], sizeof(Letter), compare_x);
    }

    // Create the sorted grid
    Letter **words_list = malloc(row_count * sizeof(Letter *));
    for (int row = 0; row < row_count; row++)
    {
        words_list[row] = malloc(col_count * sizeof(Letter));
        memcpy(words_list[row], temp_rows[row], row_sizes[row] * sizeof(Letter));
        

        for (int col = row_sizes[row]; col < col_count; col++)
        {
            words_list[row][col].x1 = 0;
            words_list[row][col].y1 = 0;
            words_list[row][col].x2 = 0;
            words_list[row][col].y2 = 0;
        }
    }

    for (int i = 0; i < row_count; i++)
    {
        free(temp_rows[i]);
    }

    *words_size_out = calloc(row_count, sizeof(int));
    for (int i = 0; i < row_count; i++)
    {
        *words_size_out[i] = row_sizes[i];
    }
    
    free(temp_rows);
    free(row_sizes);

    return words_list;
}

// Builds the list of the words
char **build_words_list(GdkPixbuf *pixbuf, Letter **words_letters, int nb_words, int *words_size)
{
    NeuralNetwork *nn = load_network(MODEL_PATH);
    if (!nn)
    {
        printf("Neural network could not be loaded, check if a model exists in "
               "tests/model\n");
        return NULL;
    }

    printf("Number of detected words : %i\n", nb_words);

    char **words_list = malloc(nb_words * sizeof(char *));
    for (int row = 0; row < nb_words; row++)
    {
        int word_size = words_size[row];
        printf("Word %i : Size %i\n", row, word_size);
        (words_list[row]) = calloc(word_size, sizeof(char));
    }

    int nb_letter = 0;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    // For each letter, determine the character with the Neural Network and add
    // it to the grid array
    for (int row = 0; row < nb_words; row++)
    {
        // printf("Row %i : Letters %i to %i :\t ", row, row * cols, row * cols
        // + cols);
        int word_size = words_size[row];
        printf("Word %i : size %i\n", row, word_size);
        for (int col = 0; col < word_size; col++)
        {
            Letter word_letter = words_letters[row][col];
            if (word_letter.x1 <= 0 || word_letter.y1 <= 0 ||
                word_letter.x2 >= width || word_letter.y2 >= height)
            {
                // printf("Failed to detect letter %i : Row : %i, col : %i\n",
                // nb_letter, row, col);
                continue;
            }
            GdkPixbuf *letter = crop(pixbuf, word_letter.x1, word_letter.y1,
                                     word_letter.x2, word_letter.y2);
            if (!letter)
            {
                // printf("No letter found at row : %i, col %i\n", row, col);
                continue;
            }
            GdkPixbuf *scaled_letter = scale_pixbuf_to_28x28(letter);

            char predicted_letter = predict_letter(nn, scaled_letter);
            // printf("%c ", predicted_letter);
            words_list[row][col] = predicted_letter;

            g_object_unref(letter);
            g_object_unref(scaled_letter);

            nb_letter++;
        }
        // printf("\n");
    }
    
    free_network(nn);

    return words_list;
}
