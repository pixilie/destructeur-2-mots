#include "../include/line_detection.h"
#include "../include/image/image.h"
#include "../include/image/image_helpers.h"
#include "../include/solver.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * sobel_filter:
 * Apply a Sobel edge-detection filter to a grayscale GdkPixbuf in-place.
 *
 * Parameters:
 *  - pixbuf: pointer to a GdkPixbuf expected to be grayscale (R==G==B).
 */
void sobel_filter(GdkPixbuf *pixbuf)
{
    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    int gx = 0;
    int gy = 0;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    int *temp_p = malloc(width * height * sizeof(int));
    for (int i = 0; i < width * height; i++)
        temp_p[i] = 0;

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    for (int i = 1; i < height - 1; i++)
    {
        for (int j = 1; j < width - 1; j++)
        {
            gx = 0;
            gy = 0;
            for (int sob_y = 0; sob_y <= 2; sob_y++)
            {
                for (int sob_x = 0; sob_x <= 2; sob_x++)
                {
                    guchar *p = pixels + (i + sob_y - 1) * rowstride +
                                (j + sob_x - 1) * n_channels;
                    gx += Gx[sob_y][sob_x] * (*p);
                    gy += Gy[sob_y][sob_x] * (*p);
                }
            }
            int diff = (int)sqrt(gx * gx + gy * gy);
            if (diff > 255)
                diff = 255;
            temp_p[i * width + j] = diff;
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            guchar *p = pixels + i * rowstride + j * n_channels;
            p[0] = p[1] = p[2] = (guchar)temp_p[i * width + j];
            if (n_channels == 4)
                p[3] = 255;
        }
    }

    free(temp_p);
}

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
 * remove_lines:
 * Detect and remove strong horizontal and vertical lines from a binarized
 * image.
 *
 * Parameters:
 *  - pixbuf: pointer to a binarized GdkPixbuf (white text on black background
 *           or inverted accordingly).
 */
void remove_lines(GdkPixbuf *pixbuf)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    for (int y = 0; y < height; y++)
    {
        int sum = 0;
        for (int x = 0; x < width; x++)
        {
            guchar *p = pixels + y * rowstride + x * n_channels;
            sum += p[0];
        }
        if (sum >= width * 255 * 0.70)
        {
            for (int x = 0; x < width; x++)
            {
                guchar *p = pixels + y * rowstride + x * n_channels;
                p[0] = p[1] = p[2] = 0;
            }
        }
    }

    for (int x = 0; x < width; x++)
    {
        int sum = 0;
        for (int y = 0; y < height; y++)
        {
            guchar *p = pixels + y * rowstride + x * n_channels;
            sum += p[0];
        }
        if (sum >= height * 255 * 0.70)
        {
            for (int y = 0; y < height; y++)
            {
                guchar *p = pixels + y * rowstride + x * n_channels;
                p[0] = p[1] = p[2] = 0;
            }
        }
    }
}

typedef struct
{
    int x;
    int y;
} Point;

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
    int min_letter_height = 10;
    int max_letter_width = 200;
    int max_letter_height = 200;

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
GridLetter *generate_letter(GdkPixbuf *pixbuf_to_crop, int *grid_coo, int **coo,
                            char *output_file, int nb_letters)
{
    g_mkdir_with_parents(output_file, 0777);

    int os = 3;
    char full_path[512];

    int letter_grid_index = 0;

    GridLetter *letters = malloc(nb_letters * sizeof(GridLetter));

    for (int index_coo = 0; index_coo < nb_letters; index_coo++)
    {
        if (coo[index_coo][0] < coo[index_coo][2] &&
            coo[index_coo][1] < coo[index_coo][3] &&
            coo[index_coo][2] - coo[index_coo][0] <= 200 &&
            coo[index_coo][3] - coo[index_coo][1] <= 200 &&
            coo[index_coo][2] - coo[index_coo][0] >= 1 &&
            coo[index_coo][3] - coo[index_coo][1] >= 10)
        {
            // If the letter is in the grid : add it to the list of grid letters
            if (coo[index_coo][0] >= grid_coo[0] &&
                coo[index_coo][1] >= grid_coo[1] &&
                coo[index_coo][2] <= grid_coo[2] &&
                coo[index_coo][3] <= grid_coo[3])
            {
                GridLetter grid_letter;
                grid_letter.x1 = coo[index_coo][0];
                grid_letter.y1 = coo[index_coo][1];
                grid_letter.x2 = coo[index_coo][2];
                grid_letter.y2 = coo[index_coo][3];
                // printf("Detected letter %i : (%i, %i)(%i, %i)\n",
                // letter_grid_index, grid_letter.x1, grid_letter.y1,
                // grid_letter.x2, grid_letter.y2);
                letters[letter_grid_index] = grid_letter;
                letter_grid_index++;
            }

            GdkPixbuf *letter = crop(
                pixbuf_to_crop, coo[index_coo][0] - os, coo[index_coo][1] - os,
                coo[index_coo][2] + os, coo[index_coo][3] + os);
            snprintf(full_path, sizeof(full_path), "%s/letter_%d_%d.png",
                     output_file, coo[index_coo][0], coo[index_coo][1]);
            save_pixbuf_as_png(letter, full_path);
            g_object_unref(letter);
        }
    }

    return letters;
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

    int *box1_coo = malloc(4 * sizeof(int));
    int *box2_coo = malloc(4 * sizeof(int));

    int box1 = 0;
    int box2 = 0;

    int seuil = 40;
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
        else if (!(coo[i][2] < box1_coo[0] - seuil ||
                   coo[i][0] > box1_coo[2] + seuil ||
                   coo[i][3] < box1_coo[1] - seuil ||
                   coo[i][1] > box1_coo[3] + seuil))
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
        else if (!(coo[i][2] < box2_coo[0] - seuil ||
                   coo[i][0] > box2_coo[2] + seuil ||
                   coo[i][3] < box2_coo[1] - seuil ||
                   coo[i][1] > box2_coo[3] + seuil))
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
    int seuilx = 50;
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

// Return the number of letters found in a bounding box (either the number of
// letters in the grid box or the number of letters in the words list box)
int count_letters_in_box(int **coo, int nb_letter, int *box)
{
    int count = 0;
    for (int i = 0; i < nb_letter; i++)
    {
        int x1 = coo[i][0];
        int y1 = coo[i][1];
        int x2 = coo[i][2];
        int y2 = coo[i][3];

        if (x2 <= x1 || y2 <= y1)
        {
            continue;
        }

        // Letter coordinates are inside the bounds of the box -> Increment
        // letter counter
        if (x1 <= box[2] && x2 >= box[0] && y1 <= box[3] && y2 >= box[1])
        {
            count++;
        }
    }
    return count;
}

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
PipelineResult pipeline(char *filename, char *output_gw_file,
                        char *output_letter_file)
{
    g_mkdir_with_parents(output_gw_file, 0777);
    g_mkdir_with_parents(output_letter_file, 0777);

    int nb_words =
        50; // we state that there will not be more than 50 words in an exercise

    PipelineResult pipelineResult;

    GdkPixbuf *pixbuf = load_image(filename);
    GdkPixbuf *pixbuf_to_slice = load_image(filename);

    save_pixbuf_as_png(pixbuf, "image.png");

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

    save_pixbuf_as_png(pixbuf, "bw.png");

    invert_color(pixbuf);

    if (best_angle != 0)
    {
        erode_3x3(pixbuf);
        median_filter_3x3(pixbuf); // Only filter level 2 images
    }

    save_pixbuf_as_png(pixbuf, "filtered.png");

    // dilate_3x3(pixbuf);

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int *grid_coo = malloc(4 * sizeof(int));
    int *words_coo = malloc(4 * sizeof(int));
    int **coo = malloc(width * height * sizeof(int *));
    for (int i = 0; i < width * height; i++)
    {
        coo[i] = malloc(4 * sizeof(int)); // coo[i][0] = x1 coo[i][1] = y1
                                          // coo[i][2] = x2 coo[i][3] = y2
        coo[i][0] = coo[i][1] = coo[i][2] = coo[i][3] = 0;
    }

    int nb_letter =
        find_letter(pixbuf, coo); // Number of letters in the grid + words list
    pipelineResult.nb_letters = nb_letter;

    find_grid_and_words(grid_coo, words_coo, coo, nb_letter);

    int nb_letter_grid = count_letters_in_box(
        coo, nb_letter, grid_coo); // Number of letters in the grid
    pipelineResult.nb_letters_grid = nb_letter_grid;
    int nb_letter_words = count_letters_in_box(
        coo, nb_letter, words_coo); // Number of letters in the words list
    pipelineResult.nb_letters_words = nb_letter_words;

    int nb_rows;
    int nb_cols;
    GridLetter *grid_letters_list = generate_letter(
        pixbuf_to_slice, grid_coo, coo, output_letter_file, nb_letter);

    GridLetter **grid_letters_array =
        build_grid_from_image(grid_letters_list, nb_letter, &nb_rows, &nb_cols);
    free(grid_letters_list);
    char **grid_array =
        build_grid_array(pixbuf_to_slice, grid_letters_array, nb_rows, nb_cols);
    for (int i = 0; i < nb_rows; i++)
    {
        free(grid_letters_array[i]);
    }
    free(grid_letters_array);

    Grid final_grid;
    final_grid.grid = grid_array;
    final_grid.nb_rows = nb_rows;
    final_grid.nb_cols = nb_cols;
    pipelineResult.grid = final_grid;

    int **word_list = malloc(nb_words * sizeof(int *));
    for (int i = 0; i < nb_words; i++)
    {
        word_list[i] = malloc(4 * sizeof(int));
        word_list[i][0] = word_list[i][1] = word_list[i][2] = word_list[i][3] =
            0;
    }

    int nb_detected_words =
        find_word_by_word(coo, word_list, words_coo, nb_letter, nb_words);
    pipelineResult.nb_words = nb_detected_words;

    // crop the grid, word list and words of the image

    char *word_path = malloc(strlen(output_gw_file) + 40 * sizeof(char));
    for (int i = 0; i < nb_words; i++)
    {
        if (word_list[i][2] - word_list[i][0] > 0 &&
            word_list[i][3] - word_list[i][1] > 0)
        {
            sprintf(word_path, "%s/word_n%i.png", output_gw_file, i + 1);
            GdkPixbuf *word =
                crop(pixbuf_to_slice, word_list[i][0], word_list[i][1],
                     word_list[i][2], word_list[i][3]);
            save_pixbuf_as_png(word, word_path);
            g_object_unref(word);
        }
    }

    char *grid_path = malloc(strlen(output_gw_file) + 40 * sizeof(char));
    char *words_path = malloc(strlen(output_gw_file) + 40 * sizeof(char));
    sprintf(grid_path, "%s/grid.png", output_gw_file);
    sprintf(words_path, "%s/words.png", output_gw_file);

    for (int i = 0; i < 4; i++)
    {
        pipelineResult.grid_coo[i] = grid_coo[i];
    }
    GdkPixbuf *grid = crop(pixbuf_to_slice, grid_coo[0], grid_coo[1],
                           grid_coo[2], grid_coo[3]);
    save_pixbuf_as_png(grid, grid_path);
    g_object_unref(grid);

    for (int i = 0; i < 4; i++)
    {
        pipelineResult.words_coo[i] = words_coo[i];
    }
    GdkPixbuf *words = crop(pixbuf_to_slice, words_coo[0], words_coo[1],
                            words_coo[2], words_coo[3]);
    save_pixbuf_as_png(words, words_path);
    g_object_unref(words);

    // #ifndef TESTING
    printf("Best rotation angle : %.2f°\n", best_angle);

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
           nb_letter, nb_letter_grid, nb_letter_words);
    printf(COLOR_YELLOW
           "[INFO]" COLOR_RESET
           " Number of words detected in the words list of the grid : %i\n",
           nb_detected_words);

    // #endif

    // free all pointers

    for (int i = 0; i < width * height; i++)
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

    free(grid_path);
    free(word_path);
    free(words_path);

    g_object_unref(pixbuf);
    g_object_unref(pixbuf_to_slice);

    return pipelineResult;
}

#ifndef TESTING
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Not enough arguments\n");
        printf("Usage: ./pipeline <input_image> <output_grid_words_dir> "
               "<output_letters_dir>\n");
        printf("\nExample:\n");
        printf("  ./pipeline input.png gw_output letters_output\n");
        printf("\nDescription:\n");
        printf("  This program processes a word search puzzle image through "
               "the OCR pipeline:\n");
        printf("   - Converts to grayscale and binarizes the image\n");
        printf("   - Finds letters and words\n");
        printf("   - Crops and saves grid, words, and individual letters\n");
        return EXIT_FAILURE;
    }

    char *input_image = argv[1];
    char *output_gw_dir = argv[2];
    char *output_letters_dir = argv[3];

    printf(COLOR_YELLOW "[INFO]" COLOR_RESET " Starting OCR pipeline...\n");
    printf("Input image: %s\n", input_image);
    printf("Output (grid/words): %s\n", output_gw_dir);
    printf("Output (letters): %s\n\n", output_letters_dir);

    pipeline(input_image, output_gw_dir, output_letters_dir);

    printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                       " Pipeline completed successfully!\n");
    printf("  Processed image saved in:\n");
    printf("   - %s (grid and word crops)\n", output_gw_dir);
    printf("   - %s (letters)\n", output_letters_dir);

    return EXIT_SUCCESS;
}
#endif
