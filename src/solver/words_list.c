#include "../../include/image_processing/image_processing.h"
#include "../../include/neural_network.h"
#include "../../include/solver/compare_letter.h"
#include "../../include/solver/letter.h"

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

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
    Letter **temp_rows = calloc(nb_letters, sizeof(Letter *));
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
char **build_words_list(NeuralNetwork *nn, GdkPixbuf *pixbuf,
                        Letter **words_letters, int nb_words, int *words_size)
{
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

    char **words_list = calloc(nb_words, sizeof(char *));
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

    printf(COLOR_YELLOW "[INFO] " COLOR_RESET
                        "Words detected in words list: %i\n",
           nb_words);
    for (int word = 0; word < nb_words; word++)
    {
        printf("Mot %i : %s\n", word + 1, words_list[word]);
    }

    return words_list;
}
