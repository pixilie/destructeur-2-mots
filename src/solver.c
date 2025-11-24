#include <ctype.h>
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
 */
int len_word(char word[])
{
    int i = 0;
    while (word[i] != '\0')
    {
        i++;
    }
    return i;
}

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
    while (ch != '\r' && ch != '\n' && ch != EOF)
    {
        nbcolls++;
        ch = fgetc(f);
    }
    rewind(f);

    char useless_line[256];
    while (fgets(useless_line, sizeof(useless_line), f) != NULL)
        nblines++;
    rewind(f);

    char tab[nblines][nbcolls];
    char line[nbcolls + 2];
    int j = 0;

    while (fgets(line, sizeof(line), f) != NULL)
    {
        for (int i = 0; i < nbcolls; i++)
        {
            if (line[i] != '\0' && line[i] != '\n' && line[i] != '\r')
                tab[j][i] = line[i];
            else
                tab[j][i] = 0;
        }
        j++;
    }
    fclose(f);

    int index_in_word = 1;
    int is_found = 0;
    int t_is_found = 0;
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
                    if (tolower(tab[l][c]) == tolower(word[0]))
                    {
                        tempx1 = c;
                        tempy1 = l;
                        tempx2 = c;
                        tempy2 = l;
                        is_found = 1;
                    }
                    c++;
                }
                if(c == nbcolls)
                {
                    l++;
                    c = 0;
                }
            }
            if (is_found == 0)
            {
                t_is_found = 1;
            }
        }
        else
        {
            int dir[8][2] = {{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1},{-1, 0},{-1, 1},{0, 1}};
            int len_w = len_word(word);
            is_found = 0;
            for(int j = 0; j < 8; j ++)
            {
                tempx2 = tempx1;
                tempy2 = tempy1;
                index_in_word = 1;
                if (tempy2 + dir[j][1] >= 0 && tempy2 + dir[j][1] < nblines && tempx2 + dir[j][0] >= 0 && tempx2 + dir[j][0] < nbcolls && is_found == 0)
                {
                    while (index_in_word < len_w && tempy2 + dir[j][1] >= 0 &&
                           tempy2 + dir[j][1] < nblines && tempx2 + dir[j][0] >= 0 &&
                           tempx2 + dir[j][0] < nbcolls && is_found == 0 &&
                           tolower(word[index_in_word]) == tolower(tab[tempy2 + dir[j][1]][tempx2+ dir[j][0]]))
                    {
                        index_in_word++;
                        tempx2 += dir[j][0];
                        tempy2 += dir[j][1];
                    }
                    if (index_in_word == len_w)
                    {
                        is_found = 1;
                    }
                }
                if (is_found == 1)
                {
                    *x2 = tempx2;
                    *y2 = tempy2;
                    *x1 = tempx1;
                    *y1 = tempy1;
                    t_is_found = 1;
                    break;
                }
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
 */
#ifndef TESTING
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
	
    if(*x1 == -1 && *x2 == -1)
    {
         printf("Not Found\n");
         exit(EXIT_FAILURE);
    }
    printf("(%i, %i)(%i, %i)\n", *x1, *y1, *x2, *y2);

    free(x1);
    free(y1);
    free(x2);
    free(y2);

    return 0;
}
#endif
