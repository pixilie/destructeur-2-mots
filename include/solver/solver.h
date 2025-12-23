#pragma once

#include "../../include/solver/letter.h"
#include "../../include/solver/compare_letter.h"
#include "../../include/solver/grid.h"
#include "../../include/solver/words_list.h"
#include "../../include/solver/words_coordinates.h"

void solve(int rows, int cols, char **grid, char word[], int *x1, int *y1,
           int *x2, int *y2);
