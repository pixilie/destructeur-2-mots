#include "../../include/solver/solver.h"

#include <ctype.h>

/*
 * solve:
 * Search for a word inside a 2D character grid. If found, write the start
 * (x1,y1) and end (x2,y2) grid coordinates. Otherwise set all outputs to -1.
 */
void solve(int rows, int cols, char **grid, char word[], int *x1, int *y1,
           int *x2, int *y2)
{
    int len = strlen(word);
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
