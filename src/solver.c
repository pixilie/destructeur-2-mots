#include "../include/image/image.h"
#include "../include/neural_network.h"
#include "image/image_helpers.h"
#include "solver.h"

#include <ctype.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Path to the model useb by the neural network to recognise letters
#define MODEL_PATH "ocr_model"

/*
 * len_word:
 * Return the length of a null-terminated string.
 */
int len_word(char word[])
{
    int i = 0;
    while (word[i] != '\0')
        i++;
    return i;
}

/*
 * solve:
 * Search for a word inside a 2D character grid. If found, write the start
 * (x1,y1) and end (x2,y2) grid coordinates. Otherwise set all outputs to -1.
 */
void solve(int rows, int cols, char **grid, char word[], int *x1, int *y1,
           int *x2, int *y2)
{
    int len = len_word(word);
    int dir[8][2] = {{1, 1},   {1, 0},  {1, -1}, {0, -1},
                     {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};

    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            if (tolower(grid[y][x]) != tolower(word[0]))
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

                    if (tolower(grid[cy][cx]) != tolower(word[i]))
                    {

                        break;
                    }

                    i++;
                }
                if (i == len) // Found word in the grid
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

    *x1 = *y1 = *x2 = *y2 = -1; // Word not found
}

/* center_x:
 * Return the x-coordinate of the center of a Letter bounding box.
 */
int center_x(const Letter *letter) { return (letter->x1 + letter->x2) / 2; }

/* center_y:
 * Return the y-coordinate of the center of a Letter bounding box.
 */
int center_y(const Letter *letter) { return (letter->y1 + letter->y2) / 2; }

/*
 * compare_x:
 * Compare two Letter instances by their center X for sorting (left-to-right).
 */
int compare_x(const void *letter_1, const void *letter_2)
{
    const Letter *letter1 = letter_1;
    const Letter *letter2 = letter_2;

    return center_x(letter1) - center_x(letter2);
}

/*
 * compare_y:
 * Compare two Letter instances by their center Y for sorting (top-to-bottom).
 */
int compare_y(const void *letter_1, const void *letter_2)
{
    const Letter *letter1 = letter_1;
    const Letter *letter2 = letter_2;

    return center_y(letter1) - center_y(letter2);
}

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
    Letter **temp_rows = malloc(nb_letters * sizeof(Letter *));
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
    Letter **grid = malloc(row_count * sizeof(Letter *));
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
char **build_grid_array(GdkPixbuf *pixbuf, Letter **grid_letters, int rows,
                        int cols, int *rows_out, int *cols_out)
{
    char *exe_dir = get_executable_dir();
    char model_path[512];
    snprintf(model_path, sizeof(model_path), "%s/../assets/%s", exe_dir,
             MODEL_PATH);

    NeuralNetwork *nn = load_network(get_image_path(model_path));
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

    char **new_grid_array = malloc(row_count * sizeof(char *));
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
    free_network(nn);

    return new_grid_array;
}

/*
 * build_words_list_from_image:
 * Group word-region Letter boxes into per-word arrays and return them.
 * Outputs: words_size_out (array of sizes), words_count_out (number of words).
 */
Letter **build_words_list_from_image(Letter *words_letters, int nb_letters,
                                     int **words_size_out, int *words_count_out)
{
    if (nb_letters <= 0 || !words_letters)
    {
        printf("No letters were detected while trying to build the words list "
               "!\n");
        *words_count_out = 0;
        *words_size_out = NULL;
        return NULL;
    }

    // Sort letters by y value
    qsort(words_letters, nb_letters, sizeof(Letter), compare_y);

    int row_threshold = 10; // The tolerance between 2 y letter values to group
                            // them in the same row
    Letter **temp_rows = malloc(nb_letters * sizeof(Letter *));
    int *row_sizes = calloc(nb_letters, sizeof(int));

    temp_rows[0] = calloc(nb_letters, sizeof(Letter));
    temp_rows[0][0] = words_letters[0];
    row_sizes[0] = 1;
    int words_count = 1; // The number of rows in the grid
    int first_letter_in_row = 0;

    for (int i = 1; i < nb_letters; i++)
    {
        int center_y_prev = center_y(&words_letters[first_letter_in_row]);
        int center_y_current = center_y(&words_letters[i]);

        // Same row
        if (abs(center_y_current - center_y_prev) <= row_threshold)
        {
            temp_rows[words_count - 1][row_sizes[words_count - 1]] =
                words_letters[i];
            row_sizes[words_count - 1]++;
        }
        // Start new row
        else
        {
            first_letter_in_row = i;
            words_count++;

            if (words_count > nb_letters)
            {
                words_count = nb_letters;
                break;
            }
            temp_rows[words_count - 1] = calloc(nb_letters, sizeof(Letter));
            temp_rows[words_count - 1][0] = words_letters[i];
            row_sizes[words_count - 1] = 1;
        }
    }

    if (words_count > nb_letters)
    {
        words_count = nb_letters;
    }

    // Sort every row by x value
    for (int row = 0; row < words_count; row++)
    {
        qsort(temp_rows[row], row_sizes[row], sizeof(Letter), compare_x);
    }

    // Create the sorted grid
    Letter **words_list = calloc(words_count, sizeof(Letter *));
    for (int word_index = 0; word_index < words_count; word_index++)
    {
        words_list[word_index] = calloc(row_sizes[word_index], sizeof(Letter));
        memcpy(words_list[word_index], temp_rows[word_index],
               row_sizes[word_index] * sizeof(Letter));
    }

    for (int i = 0; i < words_count; i++)
    {
        free(temp_rows[i]);
    }

    *words_size_out = calloc(words_count, sizeof(int));
    for (int i = 0; i < words_count; i++)
    {
        (*words_size_out)[i] = row_sizes[i];
    }

    free(temp_rows);
    free(row_sizes);
    free(words_letters);

    *words_count_out = words_count;

    return words_list;
}

/*
 * build_words_list:
 * Recognize letters for each word region using the neural network and return
 * an array of null-terminated word strings (caller must free).
 */
char **build_words_list(GdkPixbuf *pixbuf, Letter **words_letters, int nb_words,
                        int *words_size)
{
    char *exe_dir = get_executable_dir();
    char model_path[512];
    snprintf(model_path, sizeof(model_path), "%s/../assets/%s", exe_dir,
             MODEL_PATH);

    NeuralNetwork *nn = load_network(get_image_path(model_path));
    if (!nn)
    {
        printf("Neural network could not be loaded, check if a model exists in "
               "tests/model\n");
        return NULL;
    }

    if (nb_words == 0)
    {
        printf("No words were detected !\n");
        return NULL;
    }
    if (!*words_letters)
    {
        printf("No word letters detected !\n");
        return NULL;
    }
    if (!words_size)
    {
        printf("Failed to get the size of each word !\n");
        return NULL;
    }

    char **words_list = malloc(nb_words * sizeof(char *));
    for (int row = 0; row < nb_words; row++)
    {
        int word_size = words_size[row];
        words_list[row] = calloc(word_size + 1, sizeof(char));
    }

    int nb_letter = 0;
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int os = 3;

    // For each letter, determine the character with the Neural Network and add
    // it to the grid array
    for (int row = 0; row < nb_words; row++)
    {
        int word_size = words_size[row];
        for (int col = 0; col < word_size; col++)
        {
            Letter word_letter = words_letters[row][col];
            if (word_letter.x1 <= 0 || word_letter.y1 <= 0 ||
                word_letter.x2 >= width || word_letter.y2 >= height)
            {
                printf("Failed to detect word letter %i : Word : %i, word "
                       "index : %i, got coordinates : (%i, %i)(%i, %i)\n",
                       nb_letter, row, col, word_letter.x1, word_letter.y1,
                       word_letter.x2, word_letter.y2);
                nb_letter++;
                continue;
            }
            GdkPixbuf *letter =
                crop(pixbuf, word_letter.x1 - os, word_letter.y1 - os,
                     word_letter.x2 + os, word_letter.y2 + os);
            if (!letter)
            {
                printf("No word letter found at word : %i, word index %i\n",
                       row, col);
                nb_letter++;
                continue;
            }

            median_filter_3x3(letter);

            char predicted_letter = predict_letter(nn, letter);
            words_list[row][col] = predicted_letter;

            g_object_unref(letter);

            nb_letter++;
        }
        words_list[row][word_size] = '\0'; // Null terminate the word string
    }

    for (int word = 0; word < nb_words; word++)
    {
        free(words_letters[word]);
    }
    free(words_letters);
    free(words_size);
    free_network(nn);

    return words_list;
}

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
