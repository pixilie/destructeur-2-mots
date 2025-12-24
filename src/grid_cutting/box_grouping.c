#include "../../include/grid_cutting/box_grouping.h"
#include "../../include/grid_cutting/letter_sort.h"

#include <stdlib.h>

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
