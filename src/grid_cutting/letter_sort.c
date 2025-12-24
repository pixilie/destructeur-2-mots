#include "../../include/grid_cutting/letter_sort.h"

#include <stdlib.h>

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
 *  - coo: array of int* where each points to an int[4] bbox
 * [xmin,ymin,xmax,ymax]
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
