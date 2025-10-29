#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Search for a word in a rectangular letter grid stored in a text file.
 *
 * Parameters:
 *  - filename: path to the input file containing the grid (each row on a line).
 *  - word    : null-terminated string to search for.
 *  - x1, y1  : output pointers to store coordinates of the first letter
 * (start).
 *  - x2, y2  : output pointers to store coordinates of the last letter (end).
 *
 * Behavior:
 *  - Reads the grid from filename, determines number of columns from the first
 * line and number of rows from the file.
 *  - Searches for the first occurrence of word starting from the top-left cell,
 *    and looks in all 8 directions (N, E, S, W and the 4 diagonals).
 *  - Coordinates are zero-based: x is column index (0..nbcolls-1), y is row
 * index (0..nblines-1).
 *  - On success stores start (x1,y1) and end (x2,y2) coordinates via the
 * provided pointers.
 *  - If the word is not found, sets *x1 = *y1 = *x2 = *y2 = -1.
 *
 * Side effects / errors:
 *  - Exits the program with EXIT_FAILURE if the file cannot be opened.
 *  - Uses a VLA (tab[nblines][nbcolls]) to store the grid; requires that the
 * file fits memory.
 *
 * Limitations:
 *  - Assumes all grid lines have the same number of columns as the first line.
 *  - Assumes the file uses a line terminator that fgets can handle; CR/LF are
 * tolerated.
 *  - Does not validate inputs for NULL pointers.
 */
void solve(char filename[], char word[], int *x1, int *y1, int *x2, int *y2)
{
    int nbcolls = 0;
    int nblines = 0;

    int tempx1;
    int tempy1;
    int tempx2;
    int tempy2;

    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        perror("Can't open the file");
        exit(EXIT_FAILURE);
    }

    char ch = fgetc(f);
    while (ch != '\r' && ch != '\n' && ch != '\0')
    {
        ch = fgetc(f);
        nbcolls++;
    }
    rewind(f);

    char useless_line[256];
    while (fgets(useless_line, sizeof(useless_line), f) != NULL)
        nblines++;
    rewind(f);

    char tab[nblines][nbcolls];
    char line[nbcolls + 2];
    int i = 0;
    int j = 0;

    while (fgets(line, sizeof(line), f) != NULL)
    {
        while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
        {
            tab[j][i] = line[i];
            i++;
        }
        j++;
        i = 0;
    }
    fclose(f);

    int index_in_word = 1;
    int is_found = 0;
    int t_is_found = 0;
    int temp_is_found = 0;
    int c = 0;
    int l = 0;
    while (t_is_found == 0)
    {
        if (is_found == 0)
        {
            while (l < nblines && is_found == 0)
            {
                while (c < nbcolls && is_found == 0)
                {
                    if (tab[l][c] == word[0])
                    {
                        tempx1 = c;
                        tempy1 = l;
                        tempx2 = c;
                        tempy2 = l;
                        is_found = 1;
                    }
                    c++;
                }
                l++;
            }
            if (c == nbcolls && l == nblines)
            {
                t_is_found = 1;
            }
        }
        else
        {
            int len_w = strlen(word);
            is_found = 0;
            if (tempy2 - 1 >= 0)
            {
                tempy2--;
                while (index_in_word < len_w && tempy2 >= 0 &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempy2--;
                }
                if (index_in_word == len_w)
                {
                    tempy2++;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempx2 + 1 < nbcolls && is_found == 0)
            {
                tempx2++;
                while (index_in_word < len_w && tempx2 < nbcolls &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempx2++;
                }
                if (index_in_word == len_w)
                {
                    tempx2--;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempy2 + 1 < nblines && is_found == 0)
            {
                tempy2++;
                while (index_in_word < len_w && tempy2 < nblines &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempy2++;
                }
                if (index_in_word == len_w)
                {
                    tempy2--;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempx2 - 1 >= 0 && is_found == 0)
            {
                tempx2--;
                while (index_in_word < len_w && tempx2 >= 0 &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempx2--;
                }
                if (index_in_word == len_w)
                {
                    tempx2++;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempx2 + 1 < nbcolls && tempy2 + 1 < nblines && is_found == 0)
            {
                tempx2++;
                tempy2++;
                while (index_in_word < len_w && tempy2 < nblines &&
                       tempx2 < nbcolls &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempy2++;
                    tempx2++;
                }
                if (index_in_word == len_w)
                {
                    tempx2--;
                    tempy2--;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempx2 + 1 < nbcolls && tempy2 - 1 >= 0 && is_found == 0)
            {
                tempx2++;
                tempy2--;
                while (index_in_word < len_w && tempy2 >= 0 &&
                       tempx2 < nbcolls &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempy2--;
                    tempx2++;
                }
                if (index_in_word == len_w)
                {
                    tempx2--;
                    tempy2++;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempx2 - 1 >= 0 && tempy2 + 1 < nblines && is_found == 0)
            {
                tempx2--;
                tempy2++;
                while (index_in_word < len_w && tempy2 < nblines &&
                       tempx2 >= 0 &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempy2++;
                    tempx2--;
                }
                if (index_in_word == len_w)
                {
                    tempy2--;
                    tempx2++;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }
            if (tempx2 - 1 >= 0 && tempy2 - 1 >= 0 && is_found == 0)
            {
                tempx2--;
                tempy2--;
                while (index_in_word < len_w && tempy2 >= 0 && tempx2 >= 0 &&
                       word[index_in_word] == tab[tempy2][tempx2])
                {
                    index_in_word++;
                    tempy2--;
                    tempx2--;
                }
                if (index_in_word == len_w)
                {
                    tempx2++;
                    tempy2++;
                    is_found = 1;
                }
                else
                {
                    tempx2 = tempx1;
                    tempy2 = tempy1;
                    index_in_word = 1;
                    is_found = 0;
                }
            }

            if (is_found == 1)
            {
                *x2 = tempx2;
                *y2 = tempy2;
                *x1 = tempx1;
                *y1 = tempy1;
                t_is_found = 1;
            }
        }
    }
    if (is_found == 0)
    {
        *x1 = *x2 = *y1 = *y2 = -1;
    }
}

/**
 * Program entry point: parse arguments, invoke solve and print coordinates.
 *
 * Usage:
 *  ./solver filename word
 *
 * Behavior:
 *  - Validates that exactly two arguments are provided (filename and word).
 *  - Allocates temporary integers, calls solve, prints result as:
 *      ( x1 y1 )( x2 y2 )
 *  - Frees temporaries and returns 0 on success.
 *
 * Errors:
 *  - Prints an error and exits with EXIT_FAILURE when argument count is wrong.
 */
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        perror("Not enough arguments");
        exit(EXIT_FAILURE);
    }

    if (argc > 3)
    {
        perror("Too many arguments");
        exit(EXIT_FAILURE);
    }

    int *x1 = malloc(sizeof(int));
    int *y1 = malloc(sizeof(int));
    int *x2 = malloc(sizeof(int));
    int *y2 = malloc(sizeof(int));

    solve(argv[1], argv[2], x1, y1, x2, y2);

    printf("( %i %i )( %i %i )\n", *x1, *y1, *x2, *y2);

    free(x1);
    free(y1);
    free(x2);
    free(y2);

    return 0;
}
