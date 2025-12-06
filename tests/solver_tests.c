#include "../include/line_detection.h"
#include "../include/solver.h"
#include "../include/test_helpers.h"
#include <stdio.h>

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

static int rows = 9;
static int cols = 10;

char tab[9][10] = {{'H', 'O', 'R', 'I', 'Z', 'O', 'N', 'T', 'A', 'L'},
                   {'D', 'X', 'R', 'A', 'H', 'C', 'L', 'B', 'G', 'A'},
                   {'D', 'I', 'K', 'C', 'I', 'L', 'E', 'O', 'K', 'C'},
                   {'I', 'G', 'A', 'J', 'H', 'Y', 'L', 'Y', 'H', 'I'},
                   {'H', 'G', 'F', 'G', 'O', 'D', 'T', 'I', 'O', 'T'},
                   {'G', 'D', 'L', 'R', 'O', 'W', 'K', 'B', 'F', 'R'},
                   {'P', 'L', 'N', 'R', 'D', 'N', 'E', 'R', 'G', 'E'},
                   {'J', 'H', 'A', 'I', 'D', 'U', 'A', 'J', 'G', 'V'},
                   {'U', 'K', 'G', 'F', 'F', 'O', 'L', 'L', 'E', 'H'}};

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

void print_grid_array_differences(char **expected_grid_array, char **actual_grid_array, int rows, int cols, int *nb_errors_out)
{
    int nb_errors = 0;
    for (int i = 0; i < rows; i++)
    {
        printf("[");
        for (int j = 0; j < cols; j++)
        {
            if (actual_grid_array[i][j] == expected_grid_array[i][j]) // Letters equal
            {
                printf(COLOR_GREEN "%c" COLOR_RESET, actual_grid_array[i][j]);
            }
            else // Letters different
            {
                printf(COLOR_RED "%c" COLOR_RESET, actual_grid_array[i][j]);
                nb_errors++;
            }
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

    *nb_errors_out = nb_errors;
    
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
    int is_dimension_correct = 1;
    if (expected_grid.nb_rows != actual_grid.nb_rows)
    {
        print_fail();
        printf("Expected %i rows in the grid array, got %i\n",
               expected_grid.nb_rows, actual_grid.nb_rows);
        is_dimension_correct = 0;
    }
    if (expected_grid.nb_cols != actual_grid.nb_cols)
    {
        print_fail();
        printf("Expected %i columns in the grid array, got %i\n",
               expected_grid.nb_cols, actual_grid.nb_cols);
        is_dimension_correct = 0;
    }

    int rows = expected_grid.nb_rows;
    int cols = expected_grid.nb_cols;
    char **expected_grid_array = expected_grid.grid;
    char **actual_grid_array = actual_grid.grid;

    if (is_dimension_correct == 1)
    {
        print_success();
        printf("Built grid of %i rows and %i columns\n", expected_grid.nb_rows,
               expected_grid.nb_cols);
    }
    else
    {
        print_fail();
        
        printf("Expected grid :\n");
        print_grid_array(expected_grid_array, rows, cols);
        
        printf("Got grid :\n");
        print_grid_array(actual_grid_array, rows, cols);
        
        return 0;
    }

    int nb_errors = 0;
    
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (expected_grid_array[i][j] != actual_grid_array[i][j])
            {
                print_fail();
                printf("The grid arrays are different, got first letter difference detected on row %i, column %i, "
                       "expected %c, got %c\n",
                       i, j, expected_grid_array[i][j],
                       actual_grid_array[i][j]);
                
                printf("Expected grid :\n");
                print_grid_array(expected_grid_array, rows, cols);
                
                printf("Got grid :\n");
                print_grid_array_differences(expected_grid_array, actual_grid_array, rows, cols, &nb_errors);
                printf("Found %i incorrect letters in the grid\n", nb_errors);
                
                return 0;
            }
        }
    }
    print_success();
    printf("Got array of %i rows and %i columns :\n", rows, cols);
    print_grid_array(actual_grid_array, rows, cols);
    return 1;
}

void print_words_list(char **words_list, int words_count)
{
    for (int i = 0; i < words_count; i++)
    {
        printf("Word %i :\t %s\n", i, words_list[i]);
    }
}

void print_words_list_differences(char **expected_words_list, char **actual_words_list, int words_count, int *nb_errors_out)
{
    int nb_errors = 0;
    for (int i = 0; i < words_count; i++)
    {
        int expected_word_length = strlen(expected_words_list[i]);
        int actual_word_length = strlen(actual_words_list[i]);
        int min_word_length = expected_word_length < actual_word_length ? expected_word_length : actual_word_length;
        printf("Word %i :\t ", i);
        int actual_word_index = 0;
        for (int j = 0; j < min_word_length; j++)
        {
            if (expected_words_list[i][j] == actual_words_list[i][j]) // Word letter is equal
            {
                printf(COLOR_GREEN "%c" COLOR_RESET, expected_words_list[i][j]);
            }
            else // Word letter is different
            {
                printf(COLOR_RED "%c" COLOR_RESET, actual_words_list[i][j]);
                nb_errors++;
            }
            actual_word_index++;
        }
        if (actual_word_length > expected_word_length)
        {
            for (int k = actual_word_index; k < actual_word_length; k++)
            {
                printf(COLOR_RED "%c" COLOR_RESET, actual_words_list[i][k]);
                nb_errors++;
            }
        }
        printf("\n");
    }

    *nb_errors_out = nb_errors;
}

int are_words_list_equal(int expected_words_count, char **expected_words_list, int actual_words_count, char **actual_words_list)
{
    if (expected_words_count != actual_words_count)
    {
        print_fail();
        printf("Detected words count is incorrect, expected %i, got %i\n", expected_words_count, actual_words_count);
        return 0;
    }
    else
    {
        print_success();
        printf("Detected %i words in the words list\n", actual_words_count);
    }

    int nb_errors = 0;
    
    for (int i = 0; i < expected_words_count; i++)
    {
        if (strcmp(expected_words_list[i], actual_words_list[i]) != 0) // Words not equal
        {
            print_fail();
            printf("Words are incorrect, first word incorrect : word %i, expected %s, got %s\n", i, expected_words_list[i], actual_words_list[i]);
            printf("Expected words list :\n");
            print_words_list(expected_words_list, expected_words_count);
            printf("Got words list :\n");
            print_words_list_differences(expected_words_list, actual_words_list, actual_words_count, &nb_errors);
            printf("Found %i incorrect letters in the words list\n", nb_errors);
            return 0;
        }
    }
    
    print_success();
    printf("Words list is correct, got :\n");
    print_words_list(actual_words_list, actual_words_count);
    return 1;
}

int test_solver(char *test_image_name, char *filename, int nb_rows, int nb_cols, char expected_grid_array[nb_rows][nb_cols], int nb_words, char **expected_words_list)
{
    print_test_subsubcategory(test_image_name);
    
    PipelineResult pipelineResult = pipeline(filename, "gw", "letters");
    
    Grid expected_grid;
    expected_grid.nb_rows = nb_rows;
    expected_grid.nb_cols = nb_cols;

    char **expected_grid_ptr = malloc(expected_grid.nb_rows * sizeof(char *));
    for (int i = 0; i < expected_grid.nb_rows; i++)
    {
        expected_grid_ptr[i] = expected_grid_array[i];
    }

    expected_grid.grid = expected_grid_ptr;

    Grid actual_grid = pipelineResult.grid;
    Words actual_words = pipelineResult.words;
    
    char **actual_words_list = malloc(actual_words.detected_words_count * sizeof(char *));
    for (int i = 0; i < actual_words.detected_words_count; i++)
    {
        actual_words_list[i] = actual_words.words[i];
    }

    int result = 1;
    if (!are_grid_arrays_equal(expected_grid, actual_grid))
    {
        result = 0;
    }

    if (!are_words_list_equal(nb_words, expected_words_list, actual_words.detected_words_count, actual_words_list))
    {
        result = 0;
    }
    
    free(expected_grid_ptr);
    free(actual_words_list);
    return result;
}


int tests_solver()
{
    print_test_subcategory("Grid Array Tests");

    char expected_grid_array1[17][17] = {
    {'P', 'X', 'U', 'T', 'S', 'I', 'N', 'I', 'U', 'P', 'R', 'V', 'G', 'B', 'M', 'D', 'D'},
    {'E', 'H', 'A', 'A', 'S', 'P', 'O', 'J', 'P', 'E', 'T', 'B', 'E', 'Q', 'Z', 'L', 'C'},
    {'A', 'U', 'N', 'T', 'E', 'G', 'Q', 'T', 'L', 'H', 'R', 'Z', 'F', 'A', 'T', 'O', 'P'},
    {'S', 'H', 'X', 'F', 'N', 'G', 'U', 'A', 'X', 'E', 'A', 'A', 'Y', 'P', 'O', 'M', 'H'},
    {'Y', 'O', 'Y', 'Y', 'L', 'D', 'X', 'L', 'A', 'K', 'Y', 'U', 'Z', 'L', 'B', 'S', 'K'},
    {'J', 'X', 'M', 'U', 'U', 'G', 'Q', 'T', 'R', 'I', 'M', 'A', 'G', 'I', 'N', 'E', 'B'},
    {'H', 'F', 'N', 'W', 'F', 'X', 'H', 'D', 'P', 'B', 'B', 'B', 'T', 'N', 'V', 'S', 'K'},
    {'H', 'I', 'I', 'H', 'D', 'E', 'S', 'Q', 'F', 'U', 'M', 'Y', 'E', 'R', 'N', 'S', 'X'},
    {'R', 'P', 'B', 'Z', 'N', 'H', 'S', 'D', 'S', 'L', 'H', 'O', 'N', 'B', 'S', 'S', 'S'},
    {'E', 'H', 'X', 'A', 'I', 'Z', 'I', 'H', 'A', 'H', 'O', 'E', 'S', 'Q', 'F', 'E', 'F'},
    {'C', 'W', 'Z', 'I', 'M', 'V', 'D', 'C', 'J', 'V', 'S', 'S', 'I', 'M', 'G', 'R', 'W'},
    {'L', 'A', 'I', 'I', 'R', 'Z', 'Q', 'Q', 'H', 'X', 'D', 'Z', 'O', 'Z', 'Q', 'T', 'R'},
    {'W', 'C', 'A', 'X', 'E', 'Z', 'R', 'G', 'H', 'A', 'I', 'Z', 'N', 'E', 'C', 'S', 'E'},
    {'B', 'R', 'H', 'F', 'O', 'T', 'G', 'N', 'I', 'T', 'S', 'E', 'R', 'E', 'O', 'V', 'Z'},
    {'M', 'W', 'V', 'W', 'Q', 'D', 'U', 'I', 'H', 'W', 'Q', 'T', 'S', 'B', 'I', 'M', 'L'},
    {'T', 'D', 'T', 'O', 'N', 'Z', 'C', 'X', 'X', 'R', 'G', 'E', 'L', 'K', 'H', 'F', 'Q'},
    {'Q', 'N', 'E', 'K', 'S', 'V', 'M', 'O', 'T', 'F', 'A', 'L', 'A', 'A', 'E', 'W', 'B'}
};

    char expected_grid_array2[12][12] = {
    {'H', 'C', 'A', 'M', 'P', 'A', 'I', 'G', 'N', 'S', 'L', 'Z'},
    {'Y', 'C', 'P', 'H', 'Q', 'M', 'L', 'C', 'A', 'B', 'E', 'E'},
    {'U', 'W', 'O', 'R', 'K', 'D', 'R', 'I', 'V', 'E', 'K', 'T'},
    {'K', 'O', 'Y', 'O', 'D', 'Q', 'U', 'G', 'Y', 'E', 'A', 'F'},
    {'S', 'A', 'L', 'E', 'S', 'I', 'Q', 'P', 'R', 'W', 'S', 'Q'},
    {'E', 'T', 'U', 'M', 'B', 'L', 'J', 'P', 'E', 'Z', 'X', 'C'},
    {'D', 'M', 'A', 'I', 'L', 'C', 'A', 'Y', 'N', 'U', 'O', 'R'},
    {'P', 'R', 'K', 'Y', 'U', 'G', 'P', 'E', 'O', 'P', 'L', 'E'},
    {'H', 'U', 'H', 'G', 'B', 'O', 'A', 'V', 'J', 'L', 'M', 'A'},
    {'P', 'K', 'Y', 'H', 'A', 'D', 'C', 'R', 'M', 'R', 'S', 'T'},
    {'A', 'L', 'M', 'U', 'L', 'A', 'A', 'U', 'X', 'S', 'T', 'O'},
    {'K', 'R', 'U', 'B', 'O', 'O', 'K', 'S', 'A', 'T', 'O', 'R'}
};

    char expected_grid_array3[7][8] = {
    {'S', 'U', 'M', 'M', 'E', 'R', 'L', 'H'},
    {'C', 'I', 'P', 'O', 'R', 'T', 'N', 'O'},
    {'B', 'S', 'U', 'N', 'B', 'A', 'L', 'L'},
    {'R', 'E', 'L', 'A', 'X', 'E', 'P', 'I'},
    {'T', 'D', 'A', 'Q', 'S', 'A', 'N', 'D'},
    {'A', 'Y', 'B', 'C', 'A', 'Z', 'I', 'A'},
    {'N', 'F', 'U', 'N', 'H', 'R', 'S', 'Y'},
   };

    char expected_grid_array4[14][14] = {
    {'A', 'A', 'S', 'S', 'E', 'M', 'B', 'L', 'Y', 'O', 'O', 'Y', 'H', 'O'},
    {'H', 'D', 'I', 'B', 'O', 'V', 'P', 'D', 'S', 'H', 'T', 'Y', 'H', 'B'},
    {'J', 'L', 'L', 'M', 'M', 'T', 'D', 'Y', 'L', 'E', 'O', 'P', 'R', 'L'},
    {'L', 'B', 'O', 'R', 'S', 'A', 'C', 'O', 'P', 'L', 'Y', 'C', 'U', 'A'},
    {'V', 'A', 'A', 'M', 'O', 'J', 'O', 'I', 'P', 'Y', 'O', 'Y', 'S', 'Y'},
    {'T', 'S', 'M', 'A', 'I', 'W', 'S', 'M', 'S', 'T', 'L', 'R', 'T', 'A'},
    {'P', 'L', 'M', 'L', 'H', 'Y', 'S', 'J', 'S', 'A', 'L', 'O', 'A', 'V'},
    {'P', 'H', 'P', 'H', 'I', 'P', 'T', 'B', 'U', 'W', 'B', 'T', 'H', 'A'},
    {'R', 'W', 'M', 'R', 'H', 'T', 'M', 'L', 'T', 'J', 'P', 'Y', 'S', 'J'},
    {'H', 'A', 'T', 'L', 'T', 'T', 'D', 'E', 'S', 'L', 'H', 'O', 'M', 'A'},
    {'O', 'U', 'S', 'L', 'L', 'B', 'J', 'A', 'O', 'J', 'O', 'Y', 'O', 'B'},
    {'O', 'O', 'S', 'O', 'S', 'H', 'M', 'L', 'M', 'H', 'S', 'L', 'T', 'T'},
    {'S', 'R', 'T', 'A', 'P', 'Y', 'T', 'H', 'O', 'N', 'Y', 'A', 'A', 'L'},
    {'P', 'N', 'N', 'H', 'E', 'L', 'L', 'O', 'H', 'M', 'O', 'O', 'L', 'V'}
};

    char *expected_words_list_1[] = {"IMAGINE", "RELAX", "COOL", "RESTING", "BREATHE", "EASY", "TENSION", "STRESS", "CALM"};
    char *expected_words_list2[] = {"CAMPAIGNS", "ULAA", "CRM", "ONE", "DESK", "SURVEY", "CREATOR",
                                    "BOOKS", "PEOPLE", "MAIL", "SALESIQ", "WORKDRIVE", "CLIQ"};
    char *expected_words_list3[] = {"TROPIC", "BEACH", "SUMMER", "HOLIDAY", "SAND", "BALL", "TAN", "RELAX", "SUN", "FUN"};
    char *expected_words_list4[] = {"HELLO", "WORLD", "RUST", "PHP", "JAVA", "ASSEMBLY", "PYTHON", "HTML", "MOJO", "BASIC"};

    int result = 1;
    
    if (!test_solver("Level 1 Image 1", "level_1_image_1.png", 17, 17, expected_grid_array1, 9, expected_words_list_1))
    {
        result = 0;
    }

    
    if (!test_solver("Level 1 Image 2", "level_1_image_2.png", 12, 12, expected_grid_array2, 13, expected_words_list2))
    {
        result = 0;
    }
    
    if (!test_solver("Level 2 Image 1", "level_2_image_1.png", 7, 8, expected_grid_array3, 10, expected_words_list3))
    {
        result = 0;
    }
    
    if (!test_solver("Level 2 Image 2", "level_2_image_2.png", 14, 14, expected_grid_array4, 10, expected_words_list4))
    {
        result = 0;
    }
    
    
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
    if (!tests_solver())
        passed = 0;

    if (passed)
        print_all_tests_passed("Solver Tests");
    else
        print_some_tests_failed("Solver Tests");

    return passed ? 0 : 1;
}
