#include "../../include/solver/letter.h"

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
