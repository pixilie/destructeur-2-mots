#include "../include/solver.h"
#include "../include/test_helpers.h"
#include "../include/line_detection.h"
#include <stdio.h>

static int rows = 9;
static int cols = 10;

char tab[9][10] = {
    {'H','O','R','I','Z','O','N','T','A','L'},
    {'D','X','R','A','H','C','L','B','G','A'},
    {'D','I','K','C','I','L','E','O','K','C'},
    {'I','G','A','J','H','Y','L','Y','H','I'},
    {'H','G','F','G','O','D','T','I','O','T'},
    {'G','D','L','R','O','W','K','B','F','R'},
    {'P','L','N','R','D','N','E','R','G','E'},
    {'J','H','A','I','D','U','A','J','G','V'},
    {'U','K','G','F','F','O','L','L','E','H'}
};

int solve_word(char *word, int x1_out, int y1_out, int x2_out, int y2_out)
{
    int x1, y1, x2, y2;

    solve(rows, cols, tab, word, &x1, &y1, &x2, &y2);

    if (x1 != x1_out || y1 != y1_out || x2 != x2_out || y2 != y2_out)
    {
        print_fail();
        printf("Failed to solve word %s : expected (%i, %i)(%i, %i), got (%i, "
               "%i)(%i, %i)\n",
               word, x1_out, y1_out, x2_out, y2_out, x1, y1, x2, y2);
        return 0;
    }

    print_success();
    printf("Solved word %s correctly, got: ", word);

    if (x1 == -1)
        printf("Not found\n");
    else
        printf("(%i, %i)(%i, %i)\n", x1, y1, x2, y2);

    return 1;
}

int test_horizontal()
{
    print_test_subcategory("Horizontal Tests");

    int res = 1;

    res &= solve_word("horizontal", 0, 0, 9, 0);
    res &= solve_word("HORIZONTAL", 0, 0, 9, 0);
    res &= solve_word("LATNOZIROH", 9, 0, 0, 0);
    res &= solve_word("hori", 0, 0, 3, 0);

    res &= solve_word("HELLO", 9, 8, 5, 8);
    res &= solve_word("world", 5, 5, 1, 5);

    res &= solve_word("toit", 9, 4, 6, 4);
    res &= solve_word("GOD", 3, 4, 5, 4);

    res &= solve_word("EPITA", -1, -1, -1, -1); // Not found

    if (res)
    {
        print_success();
        printf("Correctly solved all horizontal words\n");
    }
    else
    {
        print_fail();
        printf("Some horizontal words were not solved correctly\n");
    }

    return res;
}

int test_vertical()
{
    print_test_subcategory("Vertical Tests");

    int res = 1;

    res &= solve_word("vertical", 9, 7, 9, 0);
    res &= solve_word("VERTICAL", 9, 7, 9, 0);
    res &= solve_word("LACITREV", 9, 0, 9, 7);

    res &= solve_word("khld", 1, 8, 1, 5);

    res &= solve_word("vetcal", -1, -1, -1, -1); // Not found

    if (res)
    {
        print_success();
        printf("Correctly solved all vertical words\n");
    }
    else
    {
        print_fail();
        printf("Some vertical words were not solved correctly\n");
    }

    return res;
}

int test_diagonal()
{
    print_test_subcategory("Diagonal Tests");

    int res = 1;

    res &= solve_word("diagonal", 0, 1, 7, 8);
    res &= solve_word("DIAGONAL", 0, 1, 7, 8);
    res &= solve_word("find", 4, 8, 1, 5);
    res &= solve_word("GOLDORAK", 8, 1, 1, 8);

    res &= solve_word("diagal", -1, -1, -1, -1); // Not found

    if (res)
    {
        print_success();
        printf("Correctly solved all diagonal words\n");
    }
    else
    {
        print_fail();
        printf("Some diagonal words were not solved correctly\n");
    }

    return res;
}

void print_grid_array(char **grid_array, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        printf("[");
        for (int j = 0; j < cols; j++)
        {
            printf("%c", grid_array[i][j]);
            if (j < cols - 1)
            {
                printf(" ");
            }
        }
        printf("]");
        if (i < rows - 1)
        {
            printf("\n");
        }
    }
    printf("\n");
}


int are_grid_arrays_equal(Grid expected_grid, Grid actual_grid)
{
    if (expected_grid.nb_rows != actual_grid.nb_rows)
    {
        print_fail();
        printf("Expected %i rows in the grid array, got %i\n", expected_grid.nb_rows, actual_grid.nb_rows);
        return 0;
    }
    if (expected_grid.nb_cols != actual_grid.nb_cols)
    {
        print_fail();
        printf("Expected %i columns in the grid array, got %i\n", expected_grid.nb_cols, actual_grid.nb_cols);
        return 0;
    }
    int rows = expected_grid.nb_rows;
    int cols = expected_grid.nb_cols;
    char **expected_grid_array = expected_grid.grid;
    char **actual_grid_array = actual_grid.grid;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (expected_grid_array[i][j] != actual_grid_array[i][j])
            {
                print_fail();
                printf("The grid arrays are different, expected \n");
                print_grid_array(expected_grid_array, rows, cols);
                printf("got\n");
                print_grid_array(actual_grid_array, rows, cols);
                return 0;
            }
        }
    }
    print_success();
    printf("Got array of %i rows and %i columns :\n", rows, cols);
    print_grid_array(actual_grid_array, rows, cols);
    return 1;
}

int test_build_grid_array()
{
    print_test_subcategory("Grid Array Tests");
    
    PipelineResult pipelineResult = pipeline("../assets/level_1_image_1.png", "gw", "letters");

    Grid expected_grid;
    expected_grid.nb_rows = 17;
    expected_grid.nb_cols = 17;
    
    char expected_grid_array[17][17] =
    {
    {'P','X','U','T','S','I','N','I','U','P','R','V', 'G', 'B', 'M', 'D', 'D'},
    {'D','X','R','A','H','C','L','B','G','A'},
    {'D','I','K','C','I','L','E','O','K','C'},
    {'I','G','A','J','H','Y','L','Y','H','I'},
    {'H','G','F','G','O','D','T','I','O','T'},
    {'G','D','L','R','O','W','K','B','F','R'},
    {'P','L','N','R','D','N','E','R','G','E'},
    {'J','H','A','I','D','U','A','J','G','V'},
    {'U','K','G','F','F','O','L','L','E','H'}
    };

    char **expected_grid_ptr = malloc(expected_grid.nb_rows * sizeof(char *));
    for (int i = 0; i < expected_grid.nb_rows; i++)
    {
        expected_grid_ptr[i] = expected_grid_array[i];
    }

    expected_grid.grid = expected_grid_ptr;
    
    Grid actual_grid = pipelineResult.grid;
    int result = are_grid_arrays_equal(expected_grid, actual_grid);
    free(expected_grid_ptr);
    return result;
}

int main()
{
    print_test_category("Solver Tests");

    int passed = 1;

    if (!test_horizontal())
        passed = 0;
    if (!test_vertical())
        passed = 0;
    if (!test_diagonal())
        passed = 0;
    if (!test_build_grid_array())
        passed = 0;

    if (passed)
        print_all_tests_passed("Solver Tests");
    else
        print_some_tests_failed("Solver Tests");

    return passed ? 0 : 1;
}
