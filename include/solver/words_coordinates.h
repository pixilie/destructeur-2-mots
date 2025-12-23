#pragma once

int **get_solved_words_grid_coos(char **words, int words_count, char **grid,
                                 int rows, int cols);
int **get_solved_words_image_coos_drawing(int **words_grid_coos,
                                          int words_count, int grid_coos[4],
                                          int rows, int cols);
