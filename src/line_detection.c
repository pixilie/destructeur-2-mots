#include "../include/image/image.h"
#include "../include/image/image_helpers.h"
#include "../include/line_detection.h"
#include "../include/solver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

/**
 * invert_color:
 * Invert the grayscale color values of a pixbuf in-place (black↔white).
 *
 * Parameters:
 *  - pixbuf: pointer to a GdkPixbuf expected to be grayscale (R==G==B).
 */
void invert_color(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            guchar *p = pixels + y * rowstride + x * n_channels;
            p[0] = p[1] = p[2] = 255 - p[0];
        }
    }
}

/**
 * find_black_pixels_around:
 * Recursive flood-fill helper that expands a connected component of white
 * pixels (considered foreground) and updates a bounding box.
 *
 * Parameters:
 *  - pixbuf     : binarized GdkPixbuf where foreground pixels have value 255.
 *  - x, y       : starting coordinates for the search.
 *  - is_visited : int array of size width*height marking visited pixels (0/1).
 *  - index_coo  : index into coo array for storing bounding box values.
 *  - coo        : array of integer arrays; coo[index_coo] holds
 * [xmin,ymin,xmax,ymax].
 */
void find_black_pixels_around(GdkPixbuf *pixbuf, int start_x, int start_y,
                              int *is_visited, int index_coo, int **coo)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    Point *queue = malloc(width * height * sizeof(Point));
    int front = 0;
    int back = 0;

    queue[back] = (Point){start_x, start_y};
    back++;

    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                            {0, 1},   {1, -1}, {1, 0},  {1, 1}};

    while (front < back)
    {
        Point point = queue[front];
        front++;

        int x = point.x;
        int y = point.y;

        guchar *p = pixels + y * rowstride + x * n_channels;

        if (p[0] < 200 || is_visited[y * width + x])
        {
            continue;
        }

        is_visited[y * width + x] = 1;

        if (x < coo[index_coo][0])
            coo[index_coo][0] = x;
        if (y < coo[index_coo][1])
            coo[index_coo][1] = y;
        if (x > coo[index_coo][2])
            coo[index_coo][2] = x;
        if (y > coo[index_coo][3])
            coo[index_coo][3] = y;

        for (int i = 0; i < 8; i++)
        {
            int nx = x + directions[i][0];
            int ny = y + directions[i][1];
            if (nx >= 0 && ny >= 0 && nx < width && ny < height)
            {
                queue[back] = (Point){nx, ny};
                back++;
            }
        }
    }

    free(queue);
}

/**
 * find_letter:
 * Find connected components representing letters in a binarized image.
 *
 * Parameters:
 *  - pixbuf: pointer to a binarized GdkPixbuf (foreground pixels = 255).
 *  - coo   : preallocated array of int* pointers; each coo[i] must point to
 *            an int[4] array where bounding boxes [xmin,ymin,xmax,ymax] are
 * stored.
 *
 * Returns:
 *  - The number of connected components (letters) discovered and written into
 * coo.
 */
int find_letter(GdkPixbuf *pixbuf, int **coo)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    int nb_letter = 0;
    int min_letter_width = 1;
    int min_letter_height = 15; // 10
    int max_letter_width = 70;
    int max_letter_height = 70;

    int *is_visited =
        malloc(width * height * sizeof(int)); // 0 if False 1 if True
    memset(is_visited, 0, width * height * sizeof(int));

    int index_coo = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (is_visited[y * width + x] == 0)
            {
                guchar *p = pixels + y * rowstride + x * n_channels;

                if (p[0] > 200)
                {
                    coo[index_coo][0] = coo[index_coo][2] = x;
                    coo[index_coo][1] = coo[index_coo][3] = y;
                    find_black_pixels_around(pixbuf, x, y, is_visited,
                                             index_coo, coo);

                    // Skip small black pixels (NOT letters)
                    if (coo[index_coo][0] < coo[index_coo][2] &&
                        coo[index_coo][1] < coo[index_coo][3] &&
                        coo[index_coo][2] - coo[index_coo][0] >=
                            min_letter_width &&
                        coo[index_coo][3] - coo[index_coo][1] >=
                            min_letter_height &&
                        coo[index_coo][2] - coo[index_coo][0] <=
                            max_letter_width &&
                        coo[index_coo][3] - coo[index_coo][1] <=
                            max_letter_height)
                    {
                        index_coo++;
                        nb_letter++;
                    }
                }
            }
        }
    }

    free(is_visited);
    return nb_letter;
}

/**
 * generate_letter:
 * Crop and save individual letter images using bounding boxes computed earlier.
 *
 * Parameters:
 *  - pixbuf_to_crop: source GdkPixbuf (original image) used for cropping.
 *  - coo            : array of bounding boxes; each coo[i] is
 * [xmin,ymin,xmax,ymax].
 *  - output_file    : directory path where letter images will be saved (created
 * if necessary).
 */
void generate_letter(int *grid_coo, int *words_coo, int **coo, int nb_letters,
                     int *nb_grid_letters_out, int *nb_words_letters_out,
                     Letter **grid_letters_out, Letter **words_letters_out)
{
    int letter_grid_count = 0;
    int letter_word_count = 0;
    Letter *grid_letters = calloc(nb_letters, sizeof(Letter));
    Letter *words_letters = calloc(nb_letters, sizeof(Letter));

    for (int index_coo = 0; index_coo < nb_letters; index_coo++)
    {
        if (coo[index_coo][0] < coo[index_coo][2] &&
            coo[index_coo][1] < coo[index_coo][3] &&
            coo[index_coo][2] - coo[index_coo][0] <= 200 && // 200
            coo[index_coo][3] - coo[index_coo][1] <= 200 && // 200
            coo[index_coo][2] - coo[index_coo][0] >= 1 &&
            coo[index_coo][3] - coo[index_coo][1] >= 10)
        {
            // If the letter is in the grid : add it to the list of grid letters
            if (coo[index_coo][0] >= grid_coo[0] &&
                coo[index_coo][1] >= grid_coo[1] &&
                coo[index_coo][2] <= grid_coo[2] &&
                coo[index_coo][3] <= grid_coo[3])
            {
                Letter grid_letter;
                grid_letter.x1 = coo[index_coo][0];
                grid_letter.y1 = coo[index_coo][1];
                grid_letter.x2 = coo[index_coo][2];
                grid_letter.y2 = coo[index_coo][3];
                grid_letters[letter_grid_count] = grid_letter;
                letter_grid_count++;
            }

            // If the letter is in the words list : add it to the list of words
            // list letters
            else if (coo[index_coo][0] >= words_coo[0] &&
                     coo[index_coo][1] >= words_coo[1] &&
                     coo[index_coo][2] <= words_coo[2] &&
                     coo[index_coo][3] <= words_coo[3])
            {
                Letter word_letter;
                word_letter.x1 = coo[index_coo][0];
                word_letter.y1 = coo[index_coo][1];
                word_letter.x2 = coo[index_coo][2];
                word_letter.y2 = coo[index_coo][3];
                words_letters[letter_word_count] = word_letter;
                letter_word_count++;
            }
        }
    }

    *nb_grid_letters_out = letter_grid_count;
    *nb_words_letters_out = letter_word_count;
    *grid_letters_out = grid_letters;
    *words_letters_out = words_letters;
}

/*
 * compare_letter_x:
 * Compare two bounding-box pointers by their x (left) coordinate.
 * Suitable for qsort when ordering letters left-to-right.
 */
int compare_letter_x(const void *a, const void *b)
{
    return (((int *)a)[0]) - (((int *)b)[0]);
}

/*
 * compare_letter_y:
 * Compare two bounding-box pointers by their y (top) coordinate.
 * Suitable for qsort when ordering letters top-to-bottom.
 */
int compare_letter_y(const void *a, const void *b)
{
    return (((int *)a)[1]) - (((int *)b)[1]);
}

/*
 * sort_letter:
 * Sort an array of letter bounding-box pointers into a reasonable reading
 * order. First sort by X (left-to-right), then for groups of close X values
 * (likely same row) sort by Y (top-to-bottom).
 *
 * Parameters:
 *  - coo: array of int* where each points to an int[4] bbox [xmin,ymin,xmax,ymax]
 *  - nb_letter: number of entries in coo
 */
void sort_letter(int **coo, int nb_letter)
{
    qsort(coo, nb_letter, sizeof(int *), compare_letter_x);
    int i = 1;
    while (i < nb_letter)
    {
        size_t j = i - 1;
        size_t tmp = 1;
        while (i < nb_letter && abs(coo[i][0] - coo[i - 1][0]) < 5)
        {
            tmp++;
            i++;
        }

        qsort(coo + j, tmp, sizeof(int *), compare_letter_y);
        i++;
    }
}

/**
 * find_grid_and_words:
 * Heuristically group detected letter bounding boxes into two clusters:
 * the grid region and the word-list region.
 *
 * Parameters:
 *  - grid_coo : int[4] output array to receive the grid bounding box.
 *  - word_coo : int[4] output array to receive the word list bounding box.
 *  - coo      : array of letter bounding boxes (coo[i] =
 * [xmin,ymin,xmax,ymax]).
 *  - nb_letter: number of entries in coo.
 */
void find_grid_and_words(int *grid_coo, int *word_coo, int **coo, int nb_letter)
{
    sort_letter(coo, nb_letter);
    int *box1_coo = malloc(4 * sizeof(int));
    int *box2_coo = malloc(4 * sizeof(int));

    int box1 = 0;
    int box2 = 0;

    int thresh = 0;

    for (int i = 0; i < nb_letter; i++)
    {
        thresh += (coo[i][3] - coo[i][1]);
    }

    thresh /= nb_letter;
    thresh *= 2;

    int threshold_b1_x = thresh;
    int threshold_b1_y = thresh;
    int threshold_b2_x = thresh;
    int threshold_b2_y = thresh;

    for (int i = 0; i < nb_letter; i++)
    {
        if (box1 == 0)
        {
            box1_coo[0] = coo[i][0];
            box1_coo[1] = coo[i][1];
            box1_coo[2] = coo[i][2];
            box1_coo[3] = coo[i][3];
            box1++;
        }
        else if (!(coo[i][2] < box1_coo[0] - threshold_b1_x ||
                   coo[i][0] > box1_coo[2] + threshold_b1_x ||
                   coo[i][3] < box1_coo[1] - threshold_b1_y ||
                   coo[i][1] > box1_coo[3] + threshold_b1_y))
        {
            box1_coo[0] = (box1_coo[0] < coo[i][0]) ? box1_coo[0] : coo[i][0];
            box1_coo[1] = (box1_coo[1] < coo[i][1]) ? box1_coo[1] : coo[i][1];
            box1_coo[2] = (box1_coo[2] > coo[i][2]) ? box1_coo[2] : coo[i][2];
            box1_coo[3] = (box1_coo[3] > coo[i][3]) ? box1_coo[3] : coo[i][3];
            box1++;
        }
        else if (box2 == 0)
        {
            box2_coo[0] = coo[i][0];
            box2_coo[1] = coo[i][1];
            box2_coo[2] = coo[i][2];
            box2_coo[3] = coo[i][3];
            box2++;
        }
        else if (!(coo[i][2] < box2_coo[0] - threshold_b2_x ||
                   coo[i][0] > box2_coo[2] + threshold_b2_x ||
                   coo[i][3] < box2_coo[1] - threshold_b2_y ||
                   coo[i][1] > box2_coo[3] + threshold_b2_y))
        {
            box2_coo[0] = (box2_coo[0] < coo[i][0]) ? box2_coo[0] : coo[i][0];
            box2_coo[1] = (box2_coo[1] < coo[i][1]) ? box2_coo[1] : coo[i][1];
            box2_coo[2] = (box2_coo[2] > coo[i][2]) ? box2_coo[2] : coo[i][2];
            box2_coo[3] = (box2_coo[3] > coo[i][3]) ? box2_coo[3] : coo[i][3];
            box2++;
        }
    }
    if (box1 < box2) // box1 = words
    {
        word_coo[0] = box1_coo[0];
        word_coo[1] = box1_coo[1];
        word_coo[2] = box1_coo[2];
        word_coo[3] = box1_coo[3];
        grid_coo[0] = box2_coo[0];
        grid_coo[1] = box2_coo[1];
        grid_coo[2] = box2_coo[2];
        grid_coo[3] = box2_coo[3];
    }
    else // box2 = words
    {
        word_coo[0] = box2_coo[0];
        word_coo[1] = box2_coo[1];
        word_coo[2] = box2_coo[2];
        word_coo[3] = box2_coo[3];
        grid_coo[0] = box1_coo[0];
        grid_coo[1] = box1_coo[1];
        grid_coo[2] = box1_coo[2];
        grid_coo[3] = box1_coo[3];
    }
    free(box1_coo);
    free(box2_coo);
}

/**
 * find_word_by_word:
 * Aggregate nearby letter bounding boxes into word boxes within the word-list
 * region.
 *
 * Parameters:
 *  - coo       : array of letter bounding boxes [xmin,ymin,xmax,ymax].
 *  - word_list : preallocated array of int* with room for nb_words entries;
 *                each entry is an int[4] that will receive grouped word boxes.
 *  - words_coo : bounding box [xmin,ymin,xmax,ymax] that defines the word-list
 * area.
 *  - nb_letter : number of letter boxes in coo.
 *  - nb_words  : capacity of word_list (maximum number of words to group).
 */
int find_word_by_word(int **coo, int **word_list, int *words_coo, int nb_letter,
                      int nb_words)
{
    int seuil = 20;
    int seuilx = 60;
    int seuily = 5;

    for (int i = 0; i < nb_letter; i++)
    {
        if (!(coo[i][2] < words_coo[0] - seuil ||
              coo[i][0] > words_coo[2] + seuil ||
              coo[i][3] < words_coo[1] - seuil ||
              coo[i][1] > words_coo[3] + seuil))
        {
            int found = 0;
            int index_word = 0;
            while (found == 0 && index_word < nb_words)
            {
                if (word_list[index_word][0] == 0 &&
                    word_list[index_word][2] == 0)
                {
                    word_list[index_word][0] = coo[i][0];
                    word_list[index_word][1] = coo[i][1];
                    word_list[index_word][2] = coo[i][2];
                    word_list[index_word][3] = coo[i][3];
                    found = 1;
                }
                else if (!(coo[i][2] < word_list[index_word][0] - seuilx ||
                           coo[i][0] > word_list[index_word][2] + seuilx) &&
                         !(coo[i][3] < word_list[index_word][1] - seuily ||
                           coo[i][1] > word_list[index_word][3] + seuily))
                {
                    word_list[index_word][0] =
                        (word_list[index_word][0] < coo[i][0])
                            ? word_list[index_word][0]
                            : coo[i][0];
                    word_list[index_word][1] =
                        (word_list[index_word][1] < coo[i][1])
                            ? word_list[index_word][1]
                            : coo[i][1];
                    word_list[index_word][2] =
                        (word_list[index_word][2] > coo[i][2])
                            ? word_list[index_word][2]
                            : coo[i][2];
                    word_list[index_word][3] =
                        (word_list[index_word][3] > coo[i][3])
                            ? word_list[index_word][3]
                            : coo[i][3];
                    found = 1;
                }
                else
                {
                    index_word++;
                }
            }
        }
    }

    int detected_words = 0;
    for (int i = 0; i < nb_words; i++)
    {
        if (word_list[i][2] - word_list[i][0] > 0 &&
            word_list[i][3] - word_list[i][1] > 0)
        {
            detected_words++;
        }
    }
    return detected_words;
}

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
PipelineResult pipeline(char *filename)
{
    int nb_words = 50; // Max number of words in the words list

    PipelineResult pipelineResult;
    GdkPixbuf *pixbuf = load_image(filename);
    GdkPixbuf *pixbuf_to_slice = load_image(filename);

    convert_to_grayscale(pixbuf);

    double best_angle = detect_best_angle(pixbuf);
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

    int *grid_coo = malloc(4 * sizeof(int));
    int *words_coo = malloc(4 * sizeof(int));

    int magic_nb_letter = 1000;

    int **coo = malloc(magic_nb_letter * sizeof(int *));
    for (int i = 0; i < magic_nb_letter; i++)
    {
        coo[i] = malloc(4 * sizeof(int)); // coo[i][0] = x1 coo[i][1] = y1
                                          // coo[i][2] = x2 coo[i][3] = y2
        coo[i][0] = coo[i][1] = coo[i][2] = coo[i][3] = 0;
    }

    int nb_letter =
        find_letter(pixbuf, coo); // Number of letters in the grid + words list
    pipelineResult.nb_letters = nb_letter;

    find_grid_and_words(grid_coo, words_coo, coo, nb_letter);

    int nb_rows;
    int nb_cols;
    int nb_letters_grid = 0;
    int nb_letters_words = 0;
    Letter *grid_letters = NULL;
    Letter *words_letters = NULL;
    generate_letter(grid_coo, words_coo, coo, nb_letter, &nb_letters_grid,
                    &nb_letters_words, &grid_letters, &words_letters);

    pipelineResult.nb_letters_grid = nb_letters_grid;
    pipelineResult.nb_letters_words = nb_letters_words;

    Letter **grid_letters_array = build_grid_from_image(
        grid_letters, nb_letters_grid, &nb_rows, &nb_cols);

    int *words_size = NULL;
    int detected_words_count = 0;

    Letter **words_letters_final = build_words_list_from_image(
        words_letters, nb_letters_words, &words_size, &detected_words_count);
    char **words_letters_list = build_words_list(
        pixbuf, words_letters_final, detected_words_count, words_size);

    pipelineResult.words.detected_words_count = detected_words_count;
    pipelineResult.words.words = words_letters_list;

    int rows;
    int cols;
    char **grid_array = build_grid_array(pixbuf, grid_letters_array, nb_rows,
                                         nb_cols, &rows, &cols);
    for (int i = 0; i < nb_rows; i++)
    {
        free(grid_letters_array[i]);
    }
    free(grid_letters_array);

    Grid final_grid;
    final_grid.grid = grid_array;
    final_grid.nb_rows = rows;
    final_grid.nb_cols = cols;
    pipelineResult.grid = final_grid;

    int **solved_words_grid_coos = get_solved_words_grid_coos(
        words_letters_list, detected_words_count, grid_array, rows, cols);
    pipelineResult.words.solved_words_grid_coos = solved_words_grid_coos;
    int **solved_words_image_coos = get_solved_words_image_coos_drawing(
        solved_words_grid_coos, detected_words_count, grid_coo, rows, cols);
    pipelineResult.words.solved_words_image_coos = solved_words_image_coos;

    int **word_list = malloc(nb_words * sizeof(int *));
    for (int i = 0; i < nb_words; i++)
    {
        word_list[i] = calloc(4, sizeof(int));
    }

    int nb_detected_words =
        find_word_by_word(coo, word_list, words_coo, nb_letter, nb_words);
    pipelineResult.nb_words = nb_detected_words;

    for (int i = 0; i < 4; i++)
    {
        pipelineResult.grid_coo[i] = grid_coo[i];
    }
    GdkPixbuf *grid = crop(pixbuf_to_slice, grid_coo[0], grid_coo[1],
                           grid_coo[2], grid_coo[3]);
    g_object_unref(grid);

    for (int i = 0; i < 4; i++)
    {
        pipelineResult.words_coo[i] = words_coo[i];
    }
    GdkPixbuf *words = crop(pixbuf_to_slice, words_coo[0], words_coo[1],
                            words_coo[2], words_coo[3]);
    g_object_unref(words);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET " Best rotation angle : %.2f°\n",
           best_angle);

    printf(COLOR_YELLOW "[INFO][SOLVER]" COLOR_RESET
                        " Built grid with %i rows and %i columns\n",
           pipelineResult.grid.nb_rows, pipelineResult.grid.nb_cols);
    printf(COLOR_YELLOW "[INFO][SOLVER]" COLOR_RESET
                        " Built words list with %i words detected\n",
           pipelineResult.words.detected_words_count);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET
                        " Detected grid coordinates : (%i, %i)(%i, %i)\n",
           pipelineResult.grid_coo[0], pipelineResult.grid_coo[1],
           pipelineResult.grid_coo[2], pipelineResult.grid_coo[3]);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET
                        " Detected words list coordinates : (%i, %i)(%i, %i)\n",
           pipelineResult.words_coo[0], pipelineResult.words_coo[1],
           pipelineResult.words_coo[2], pipelineResult.words_coo[3]);

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET
                        " Number of letters detected : %i (In the grid : %i, "
                        "in the words list : %i)\n",
           pipelineResult.nb_letters, pipelineResult.nb_letters_grid,
           pipelineResult.nb_letters_words);
    printf(COLOR_YELLOW
           "[INFO]" COLOR_RESET
           " Number of words detected in the words list of the grid : %i\n",
           pipelineResult.nb_words);

    Words words_pipeline = pipelineResult.words;
    for (int i = 0; i < words_pipeline.detected_words_count; i++)
    {
        free(words_pipeline.solved_words_grid_coos[i]);
        free(words_pipeline.solved_words_image_coos[i]);
        free(words_pipeline.words[i]);
    }
    free(words_pipeline.solved_words_grid_coos);
    free(words_pipeline.solved_words_image_coos);
    free(words_pipeline.words);

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
