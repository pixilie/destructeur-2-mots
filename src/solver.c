#include <ctype.h>
#include <stdio.h>

/**
 * Return the length of a string (no strlen allowed).
 */
static int len_word(char word[])
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
