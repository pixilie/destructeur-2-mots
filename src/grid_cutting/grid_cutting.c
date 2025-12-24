#include "../include/grid_cutting.h"
#include "../include/image_processing/image_processing.h"
#include "../include/neural_network.h"
#include "../include/solver/grid.h"
#include "../include/solver/letter.h"
#include "../include/solver/words_coordinates.h"
#include "../include/solver/words_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

/**
 * pipeline:
 * High-level OCR pipeline that processes a puzzle image to extract grid, words,
 * and letters.
 *
 * Parameters:
 *  - filename         : input image filename (asset name passed to load_image).
 *  - output_gw_file   : output directory for grid and word crops.
 *  - output_letter_file: output directory for individual letter images.
 */
PipelineResult *pipeline(char *filename, NeuralNetwork *nn)
{
    if (!nn)
    {
        printf(COLOR_RED "[ERREUR] " COLOR_RESET
                         "Le réseau de neurones n'a pas pu être chargé\n");
    }

    int nb_words = 50; // Max number of words in the words list

    PipelineResult *pipelineResult = calloc(1, sizeof(PipelineResult));
    if (!pipelineResult)
    {
        printf("Failed to allocate pipelineResult\n");
        return NULL;
    }

    GdkPixbuf *pixbuf = load_image(filename);
    GdkPixbuf *pixbuf_to_slice = load_image(filename);

    convert_to_grayscale(pixbuf);

    double best_angle = detect_best_angle(pixbuf);
    pipelineResult->rotation_angle = best_angle;
    if (best_angle != 0)
    {
        GdkPixbuf *rotated = rotate_image(pixbuf, best_angle);
        g_object_unref(pixbuf);
        pixbuf = rotated;

        GdkPixbuf *rotated_slice = rotate_image(pixbuf_to_slice, best_angle);
        g_object_unref(pixbuf_to_slice);
        pixbuf_to_slice = rotated_slice;
    }

    convert_to_black_and_white(pixbuf);
    invert_color(pixbuf);

    int *grid_coo = calloc(4, sizeof(int));
    int *words_coo = calloc(4, sizeof(int));

    int magic_nb_letter = 1000;

    int **coo = calloc(magic_nb_letter, sizeof(int *));
    for (int i = 0; i < magic_nb_letter; i++)
    {
        coo[i] = calloc(4, sizeof(int)); // coo[i][0] = x1 coo[i][1] = y1
                                         // coo[i][2] = x2 coo[i][3] = y2
    }

    int nb_letter =
        find_letter(pixbuf, coo); // Number of letters in the grid + words list
    pipelineResult->nb_letters = nb_letter;

    find_grid_and_words(grid_coo, words_coo, coo, nb_letter);

    int nb_rows;
    int nb_cols;
    int nb_letters_grid = 0;
    int nb_letters_words = 0;
    Letter *grid_letters = NULL;
    Letter *words_letters = NULL;
    generate_letter(grid_coo, words_coo, coo, nb_letter, &nb_letters_grid,
                    &nb_letters_words, &grid_letters, &words_letters);

    pipelineResult->nb_letters_grid = nb_letters_grid;
    pipelineResult->nb_letters_words = nb_letters_words;

    Letter **grid_letters_array = build_grid_from_image(
        grid_letters, nb_letters_grid, &nb_rows, &nb_cols);

    int *words_size = NULL;
    int detected_words_count = 0;

    Letter **words_letters_final = build_words_list_from_image(
        words_letters, nb_letters_words, &words_size, &detected_words_count);
    char **words_letters_list = build_words_list(
        nn, pixbuf, words_letters_final, detected_words_count, words_size);

    pipelineResult->words.detected_words_count = detected_words_count;
    pipelineResult->words.words = words_letters_list;

    int rows;
    int cols;
    char **grid_array = build_grid_array(nn, pixbuf, grid_letters_array,
                                         nb_rows, nb_cols, &rows, &cols);
    for (int i = 0; i < nb_rows; i++)
    {
        free(grid_letters_array[i]);
    }
    free(grid_letters_array);

    Grid final_grid;
    final_grid.grid = grid_array;
    final_grid.nb_rows = rows;
    final_grid.nb_cols = cols;
    pipelineResult->grid = final_grid;

    int **solved_words_grid_coos = get_solved_words_grid_coos(
        words_letters_list, detected_words_count, grid_array, rows, cols);
    pipelineResult->words.solved_words_grid_coos = solved_words_grid_coos;
    int **solved_words_image_coos = get_solved_words_image_coos_drawing(
        solved_words_grid_coos, detected_words_count, grid_coo, rows, cols);
    pipelineResult->words.solved_words_image_coos = solved_words_image_coos;

    int **word_list = calloc(nb_words, sizeof(int *));
    for (int i = 0; i < nb_words; i++)
    {
        word_list[i] = calloc(4, sizeof(int));
    }

    int nb_detected_words =
        find_word_by_word(coo, word_list, words_coo, nb_letter, nb_words);
    pipelineResult->nb_words = nb_detected_words;

    for (int i = 0; i < 4; i++)
    {
        pipelineResult->grid_coo[i] = grid_coo[i];
    }
    GdkPixbuf *grid = crop(pixbuf_to_slice, grid_coo[0], grid_coo[1],
                           grid_coo[2], grid_coo[3]);
    g_object_unref(grid);

    for (int i = 0; i < 4; i++)
    {
        pipelineResult->words_coo[i] = words_coo[i];
    }
    GdkPixbuf *words = crop(pixbuf_to_slice, words_coo[0], words_coo[1],
                            words_coo[2], words_coo[3]);
    g_object_unref(words);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET " Best rotation angle : %.2f°\n",
           best_angle);

    printf(COLOR_YELLOW "[INFO][SOLVER]" COLOR_RESET
                        " Built grid with %i rows and %i columns\n",
           pipelineResult->grid.nb_rows, pipelineResult->grid.nb_cols);
    printf(COLOR_YELLOW "[INFO][SOLVER]" COLOR_RESET
                        " Built words list with %i words detected\n",
           pipelineResult->words.detected_words_count);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET
                        " Detected grid coordinates : (%i, %i)(%i, %i)\n",
           pipelineResult->grid_coo[0], pipelineResult->grid_coo[1],
           pipelineResult->grid_coo[2], pipelineResult->grid_coo[3]);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET
                        " Detected words list coordinates : (%i, %i)(%i, %i)\n",
           pipelineResult->words_coo[0], pipelineResult->words_coo[1],
           pipelineResult->words_coo[2], pipelineResult->words_coo[3]);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET
                        " Number of letters detected : %i (In the grid : %i, "
                        "in the words list : %i)\n",
           pipelineResult->nb_letters, pipelineResult->nb_letters_grid,
           pipelineResult->nb_letters_words);
    printf(COLOR_YELLOW
           "[INFO]" COLOR_RESET
           " Number of words detected in the words list of the grid : %i\n",
           pipelineResult->nb_words);

    for (int i = 0; i < magic_nb_letter; i++)
    {
        free(coo[i]);
    }
    for (int i = 0; i < nb_words; i++)
    {
        free(word_list[i]);
    }
    free(coo);
    free(word_list);
    free(grid_coo);
    free(words_coo);

    g_object_unref(pixbuf);
    g_object_unref(pixbuf_to_slice);

    return pipelineResult;
}

void free_pipeline(PipelineResult *pipelineResult)
{
    Words words_pipeline = pipelineResult->words;
    for (int i = 0; i < words_pipeline.detected_words_count; i++)
    {
        free(words_pipeline.solved_words_grid_coos[i]);
        free(words_pipeline.solved_words_image_coos[i]);
        free(words_pipeline.words[i]);
    }
    free(words_pipeline.solved_words_grid_coos);
    free(words_pipeline.solved_words_image_coos);
    free(words_pipeline.words);

    for (int i = 0; i < pipelineResult->grid.nb_rows; i++)
    {
        free(pipelineResult->grid.grid[i]);
    }
    free(pipelineResult->grid.grid);

    free(pipelineResult);
}
