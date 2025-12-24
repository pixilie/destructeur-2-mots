#include "../../include/grid_cutting/word_detection.h"

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
