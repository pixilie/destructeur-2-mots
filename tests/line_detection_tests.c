#include "../include/line_detection.h"
#include "../include/test_helpers.h"

int are_coordinates_correct(char *coo_name, int expected_coo[4],
                            int actual_coo[4])
{
    int is_correct = 1;
    for (int i = 0; i < 4; i++)
    {
        if (expected_coo[i] != actual_coo[i])
        {
            is_correct = 0;
            break;
        }
    }
    if (is_correct == 0)
    {
        print_fail();
        printf("%s coordinates are incorrect, expected : (%i, %i)(%i, %i), got "
               ": (%i, %i)(%i, "
               "%i)\n",
               coo_name, expected_coo[0], expected_coo[1], expected_coo[2],
               expected_coo[3], actual_coo[0], actual_coo[1], actual_coo[2],
               actual_coo[3]);
        return 0;
    }
    else
    {
        print_success();
        printf("%s coordinates are correct, got : (%i, %i)(%i, %i)\n", coo_name,
               expected_coo[0], expected_coo[1], expected_coo[2],
               expected_coo[3]);
        return 1;
    }
}

int is_count_correct(char *count_name, int expected_count, int actual_count)
{
    if (expected_count == actual_count)
    {
        print_success();
        printf("%s count is correct, got : %i\n", count_name, expected_count);
        return 1;
    }
    else
    {
        print_fail();
        printf("%s count is incorrect, expected : %i, got : %i\n", count_name,
               expected_count, actual_count);
        return 0;
    }
}

int test_pipeline(char *test_name, char *filename, char *output_gw_file,
                  char *output_letter_file, int grid_coo[4], int words_coo[4],
                  int nb_letters, int nb_letters_grid, int nb_letters_words,
                  int nb_words)
{
    print_test_subcategory(test_name);

    int result = 1;

    PipelineResult pipelineResult =
        pipeline(filename, output_gw_file, output_letter_file);

    result &=
        are_coordinates_correct("Grid", grid_coo, pipelineResult.grid_coo);
    result &=
        are_coordinates_correct("Words", words_coo, pipelineResult.words_coo);
    result &=
        is_count_correct("Letters", nb_letters, pipelineResult.nb_letters);
    result &= is_count_correct("Grid letters", nb_letters_grid,
                               pipelineResult.nb_letters_grid);
    result &= is_count_correct("Words letters", nb_letters_words,
                               pipelineResult.nb_letters_words);
    result &= is_count_correct("Words", nb_words, pipelineResult.nb_words);

    return result;
}

int main()
{
    print_test_category("Pipeline Tests");
    int result = 1;

    // Level 1 Image 1
    int grid_coo1[4] = {195, 30, 766, 603};
    int words_coo1[4] = {24, 123, 133, 398};
    int nb_letters1 = 340;
    int nb_letters_grid1 = 289;
    int nb_letters_words1 = 51;
    int nb_words1 = 9;
    if (!test_pipeline("Level 1 Image 1", "../assets/level_1_image_1.png",
                       "tests/results/gw1", "tests/results/letters1", grid_coo1,
                       words_coo1, nb_letters1, nb_letters_grid1,
                       nb_letters_words1, nb_words1))
    {
        result = 0;
    }

    // Level 1 Image 2
    int grid_coo2[4] = {31, 33, 638, 643};
    int words_coo2[4] = {713, 54, 876, 641};
    int nb_letters2 = 215;
    int nb_letters_grid2 = 144;
    int nb_letters_words2 = 71;
    int nb_words2 = 13;
    if (!test_pipeline("Level 1 Image 2", "../assets/level_1_image_2.png",
                       "tests/results/gw2", "tests/results/letters2", grid_coo2,
                       words_coo2, nb_letters2, nb_letters_grid2,
                       nb_letters_words2, nb_words2))
    {
        result = 0;
    }

    // Level 2 Image 1
    int grid_coo3[4] = {184, 18, 776, 610};
    int words_coo3[4] = {975, 360, 1117, 803};
    int nb_letters3 = 102;
    int nb_letters_grid3 = 56;
    int nb_letters_words3 = 46;
    int nb_words3 = 10;
    if (!test_pipeline("Level 2 Image 1", "../assets/level_2_image_1.png",
                       "tests/results/gw3", "tests/results/letters3", grid_coo3,
                       words_coo3, nb_letters3, nb_letters_grid3,
                       nb_letters_words3, nb_words3))
    {
        result = 0;
    }

    // Level 2 Image 2
    int grid_coo4[4] = {88, 97, 813, 831};
    int words_coo4[4] = {871, 215, 1175, 700};
    int nb_letters4 = 244;
    int nb_letters_grid4 = 196;
    int nb_letters_words4 = 48;
    int nb_words4 = 10;
    if (!test_pipeline("Level 2 Image 2", "../assets/level_2_image_2.png",
                       "tests/results/gw4", "tests/results/letters4", grid_coo4,
                       words_coo4, nb_letters4, nb_letters_grid4,
                       nb_letters_words4, nb_words4))
    {
        result = 0;
    }

    if (result == 1)
    {
        print_all_tests_passed("Pipeline Tests");
    }
    else
    {
        print_some_tests_failed("Pipeline Tests");
    }

    return result;
}
