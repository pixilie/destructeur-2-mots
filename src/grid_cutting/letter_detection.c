#include "../../include/grid_cutting/letter_detection.h"
#include "../../include/grid_cutting/connected_component_box.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdlib.h>


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
                    find_white_pixels_around(pixbuf, x, y, is_visited,
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
