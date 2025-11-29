#include "../../include/image/image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Terminal color codes for pretty output
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

void print_all_function_usages()
{
    printf("Usage: /image <function_name> <image_path> <function_params> "
           "<optional:output>\n");
    printf("Available functions:\n");
    printf(" - ./image convert_to_grayscale <image_path> <optional:output>\n");
    printf(" - ./image binarize_image <image_path> <threshold> "
           "<optional:output>\n");
    printf(" - ./image convert_to_black_and_white <image_path> "
           "<optional:output>\n");
    printf(" - ./image median_filter_3x3 <image_path>\n");
    printf(" - ./image rotate_image <image_path> <angle_degrees> "
           "<optional:output>\n");
    printf(" - ./image rotate_image_automatic <image_path> "
           "<optional: output>\n");
    printf(" - ./image slice_from <image_path> <x> <y> <direction> "
           "<optional:output1> <optional:output2>\n");
    printf(" - ./image slice_in_n <image_path> <n_slice> "
           "<optional:output_prefix>\n");
    printf(" - ./image crop <image_path> <x1> <y1> <x2> <y2> "
           "<optional:output>\n");
}

int image_convert_to_grayscale(int argc, char **argv)
{
    if (argc < 3)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for convert_to_grayscale\n");
        printf("Usage: ./image convert_to_grayscale <image_path> "
               "<optional:output>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    convert_to_grayscale(pixbuf);
    if (argc > 3)
    {
        save_pixbuf_as_png(pixbuf, argv[3]);
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s converted to grayscale and saved as %s\n",
               argv[2], argv[3]);
    }
    else
    {
        save_pixbuf_as_png(pixbuf, "grayscaled.png");
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s converted to grayscale and saved "
                           "as grayscaled.png\n",
               argv[2]);
    }
    g_object_unref(pixbuf);
    return EXIT_SUCCESS;
}

int image_binarize_image(int argc, char **argv)
{
    if (argc < 4)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for binarize_image\n");
        printf("Usage: ./image binarize_image <image_path> "
               "<threshold> <optional:output>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    int threshold = strtol(argv[3], NULL, 10);
    binarize_image(pixbuf, threshold);
    if (argc > 4)
    {
        save_pixbuf_as_png(pixbuf, argv[4]);
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s converted to black and white "
                           "with threshold %i and saved as %s\n",
               argv[2], threshold, argv[4]);
    }
    else
    {
        save_pixbuf_as_png(pixbuf, "binarized.png");
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET
               " Image %s converted to black and white with threshold "
               "%i and saved as binarized.png\n",
               argv[2], threshold);
    }
    g_object_unref(pixbuf);
    return EXIT_SUCCESS;
}

int image_convert_to_black_and_white(int argc, char **argv)
{
    if (argc < 3)
    {
        printf(COLOR_RED
               "[FAIL]" COLOR_RESET
               " Not enough arguments for convert_to_black_and_white\n");
        printf("Usage: ./image convert_to_black_and_white <image_path>"
               " <optional:output>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    convert_to_grayscale(pixbuf);
    int threshold = convert_to_black_and_white(pixbuf);
    if (argc > 3)
    {
        save_pixbuf_as_png(pixbuf, argv[3]);
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET
               " Image %s converted to black and white with threshold"
               " %i and saved as %s\n",
               argv[2], threshold, argv[3]);
    }
    else
    {
        save_pixbuf_as_png(pixbuf, "black_and_white.png");
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET " Image %s converted to black and white"
               " with threshold %i and saved as black_and_white.png\n",
               argv[2], threshold);
    }
    g_object_unref(pixbuf);
    return EXIT_SUCCESS;
}

int image_median_filter_3x3(int argc, char **argv)
{
    if (argc < 3)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for median_filter_3x3\n");
        printf("Usage: ./image median_filter_3x3 <image_path>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    median_filter_3x3(pixbuf);
    if (argc > 3)
    {
        save_pixbuf_as_png(pixbuf, argv[3]);
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET
               " Image %s filtered with median filter and saved as %s\n",
               argv[2], argv[3]);
    }
    else
    {
        save_pixbuf_as_png(pixbuf, "filtered.png");
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s filtered with median filter and "
                           "saved as filtered.png\n",
               argv[2]);
    }
    g_object_unref(pixbuf);
    return EXIT_SUCCESS;
}

int image_rotate_image(int argc, char **argv)
{
    if (argc < 4)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for rotate_image\n");
        printf("Usage: ./image rotate_image <image_path> "
               "<angle_degrees> <optional:output>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    double angle_degrees = strtod(argv[3], NULL);
    GdkPixbuf *rotated = rotate_image(pixbuf, angle_degrees);
    if (!rotated)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Rotate function failed\n");
        g_object_unref(pixbuf);
        return EXIT_FAILURE;
    }
    if (argc > 4)
    {
        save_pixbuf_as_png(rotated, argv[4]);
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET
               " Image %s rotated by %.2f degrees and saved as %s\n",
               argv[2], angle_degrees, argv[4]);
    }
    else
    {
        save_pixbuf_as_png(rotated, "rotated.png");
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s rotated by %.2f degrees and "
                           "saved as rotated.png\n",
               argv[2], angle_degrees);
    }
    g_object_unref(pixbuf);
    g_object_unref(rotated);
    return EXIT_SUCCESS;
}

int image_rotate_image_automatic(int argc, char **argv)
{
    if (argc < 3)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for rotate_image_automatic\n");
        printf("Usage: ./image rotate_image_automatic <image_path> "
               "<optional: output>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    double best_angle = detect_best_angle(pixbuf);
    if (best_angle == 0)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         "Image is already upright, no rotation applied !\n");
        g_object_unref(pixbuf);
        return EXIT_FAILURE;
    }

    GdkPixbuf *rotated_automatic = rotate_image_automatic(pixbuf);
    if (!rotated_automatic)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Rotate automatic function failed\n");
        g_object_unref(pixbuf);
        return EXIT_FAILURE;
    }

    if (argc > 3)
    {
        save_pixbuf_as_png(rotated_automatic, argv[3]);
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s rotated automatically by %.2f "
                           "degrees and saved as %s\n",
               argv[2], best_angle, argv[3]);
    }
    else
    {
        save_pixbuf_as_png(rotated_automatic, "rotated_automatic.png");
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET
               " Image %s rotated automatically by %.2f degrees and "
               "saved as rotated_automatic.png\n",
               argv[2], best_angle);
    }
    g_object_unref(pixbuf);
    g_object_unref(rotated_automatic);
    return EXIT_SUCCESS;
}

int image_slice_from(int argc, char **argv)
{
    if (argc < 6)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for slice_from\n");
        printf("Usage: ./image slice_from <image_path> <x> <y> "
               "<direction> <optional:output1> <optional:output2>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    int x = (int)strtol(argv[3], NULL, 10);
    int y = (int)strtol(argv[4], NULL, 10);
    int direction = (int)strtol(argv[5], NULL, 10);
    GdkPixbuf **pixbufs = slice_from(pixbuf, x, y, direction);
    if (!pixbufs)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Slice_from failed\n");
        g_object_unref(pixbuf);
        return EXIT_FAILURE;
    }

    GdkPixbuf *pixbuf1 = pixbufs[0];
    GdkPixbuf *pixbuf2 = pixbufs[1];
    if (direction == 1) // horiontal cut
    {
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s has been sliced horizontally at "
                           "y = %i into 2 smaller images\n",
               argv[2], y);
    }
    else // vertical cut
    {
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s has been sliced vertically at x "
                           "= %i into 2 smaller images\n",
               argv[2], x);
    }

    if (argc > 6)
    {
        save_pixbuf_as_png(pixbuf1, argv[6]);
        if (direction == 1)
        {
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                               " Top image has been saved as %s\n",
                   argv[6]);
        }
        else
        {
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                               " Left image has been saved as %s\n",
                   argv[6]);
        }
        if (argc > 7)
        {
            save_pixbuf_as_png(pixbuf2, argv[7]);
            if (direction == 1)
            {
                printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                                   " Bottom image has been saved as %s\n",
                       argv[7]);
            }
            else
            {
                printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                                   " Right image has been saved as %s\n",
                       argv[7]);
            }
        }
        else
        {
            if (direction == 1)
            {
                save_pixbuf_as_png(pixbuf2, "sliced_image_bottom.png");
                printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                                   " Bottom image has been saved as "
                                   "sliced_image_bottom.png\n");
            }
            else
            {
                save_pixbuf_as_png(pixbuf2, "sliced_image_right.png");
                printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                                   " Right image has been saved as "
                                   "sliced_image_right.png\n");
            }
        }
    }
    else
    {
        if (direction == 1)
        {
            save_pixbuf_as_png(pixbuf1, "sliced_image_top.png");
            printf(COLOR_GREEN
                   "[SUCCESS]" COLOR_RESET
                   " Top image has been saved as sliced_image_top.png\n");
            save_pixbuf_as_png(pixbuf2, "sliced_image_bottom.png");
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                               " Bottom image has been saved as "
                               "sliced_image_bottom.png\n");
        }
        else
        {
            save_pixbuf_as_png(pixbuf1, "sliced_image_left.png");
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                               " Left image has been saved as "
                               "sliced_image_left.png\n");
            save_pixbuf_as_png(pixbuf2, "sliced_image_right.png");
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                               " Right image has been saved as "
                               "sliced_image_right.png\n");
        }
    }
    g_object_unref(pixbuf);
    g_object_unref(pixbufs[0]);
    g_object_unref(pixbufs[1]);
    free(pixbufs);
    return EXIT_SUCCESS;
}

int image_slice_in_n(int argc, char **argv)
{
    if (argc < 4)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for slice_in_n\n");
        printf("Usage: ./image slice_in_n <image_path> <n_slice> "
               "<optional:output_prefix>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    int n_slice = (int)strtol(argv[3], NULL, 10);
    GdkPixbuf **tiles = slice_in_n(pixbuf, n_slice);
    if (!tiles)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Slice_in_n failed\n");
        g_object_unref(pixbuf);
        return EXIT_FAILURE;
    }
    printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                       " Image %s has been sliced into %i tiles\n",
           argv[2], n_slice * n_slice);

    const char *prefix;
    if (argc > 4)
    {
        prefix = argv[4];
    }
    else
    {
        prefix = "sliced_in_n";
    }

    char filename[256];
    int index = 0;
    for (int i = 0; i < n_slice; i++)
    {
        for (int j = 0; j < n_slice; j++)
        {
            snprintf(filename, sizeof(filename), "%s_%i_%i.png", prefix, i, j);
            save_pixbuf_as_png(tiles[index], filename);
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                               " Tile [%i, %i] has been saved as %s\n",
                   i, j, filename);
            g_object_unref(tiles[index]);
            index++;
        }
    }

    free(tiles);
    g_object_unref(pixbuf);
    return EXIT_SUCCESS;
}

int image_crop(int argc, char **argv)
{
    if (argc < 7)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Not enough arguments for crop\n");
        printf("Usage: ./image crop <image_path> <x1> <y1> <x2> <y2> "
               "<optional:output_prefix>\n");
        return EXIT_FAILURE;
    }

    // Load image
    GdkPixbuf *pixbuf = load_image(argv[2]);
    if (!pixbuf)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Image could not be loaded\n");
        return EXIT_FAILURE;
    }

    int x1 = (int)strtol(argv[3], NULL, 10);
    int y1 = (int)strtol(argv[4], NULL, 10);
    int x2 = (int)strtol(argv[5], NULL, 10);
    int y2 = (int)strtol(argv[6], NULL, 10);
    GdkPixbuf *cropped = crop(pixbuf, x1, y1, x2, y2);
    if (!cropped)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Crop failed\n");
        g_object_unref(pixbuf);
        return EXIT_FAILURE;
    }
    if (argc > 7)
    {
        save_pixbuf_as_png(cropped, argv[7]);
        printf(COLOR_GREEN
               "[SUCCESS]" COLOR_RESET
               " Image %s cropped into a smaller image of rectangle "
               "(x1:%i, y1:%i) -> (x2:%i, y2:%i) and saved as %s\n",
               argv[2], x1, y1, x2, y2, argv[7]);
    }
    else
    {
        save_pixbuf_as_png(cropped, "cropped.png");
        printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET
                           " Image %s cropped into a smaller image of "
                           "rectangle (x1:%i, y1:%i) -> (x2:%i, y2:%i) "
                           "and saved as cropped.png\n",
               argv[2], x1, y1, x2, y2);
    }
    g_object_unref(pixbuf);
    g_object_unref(cropped);
    return EXIT_SUCCESS;
}

#ifndef TESTING
/**
 * main:
 * Command-line driver for image utility functions. Parses arguments, loads the
 * requested image from the assets directory and dispatches to the requested
 * image operation (convert_to_grayscale, binarize_image, rotate_image,
 * slice_from, slice_in_n, crop). Saves outputs when requested and prints
 * status messages to stdout.
 *
 * Parameters:
 *  - argc: number of command-line arguments.
 *  - argv: argument vector. Expected forms:
 *      ./image convert_to_grayscale <image_path> [output]
 *      ./image binarize_image <image_path> <threshold> [output]
 *      ./image convert_to_black_and_white <image_path> [output]
 *      ./image median_filter_3x3 <image_path>
 *      ./image rotate_image <image_path> <angle_degrees> [output]
 *      ./image rotate_image_automatic <image_path> [output]
 *      ./image slice_from <image_path> <x> <y> <direction> [out1] [out2]
 *      ./image slice_in_n <image_path> <n_slice> [output_prefix]
 *      ./image crop <image_path> <x1> <y1> <x2> <y2> [output]
 */
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET " Not enough arguments\n");
        printf("Please specify a function name\n");
        print_all_function_usages();
        return EXIT_FAILURE;
    }

    char *function_name = argv[1];

    // Check function name
    if (strcmp(function_name, "convert_to_grayscale") &&
        strcmp(function_name, "binarize_image") &&
        strcmp(function_name, "convert_to_black_and_white") &&
        strcmp(function_name, "median_filter_3x3") &&
        strcmp(function_name, "rotate_image") &&
        strcmp(function_name, "rotate_image_automatic") &&
        strcmp(function_name, "slice_from") &&
        strcmp(function_name, "slice_in_n") && strcmp(function_name, "crop"))
    {
        printf(COLOR_RED "[FAIL]" COLOR_RESET
                         " Incorrect function name, got: " COLOR_RED
                         "%s\n" COLOR_RESET,
               function_name);
        print_all_function_usages();
        return EXIT_FAILURE;
    }

    // convert_to_grayscale
    if (strcmp(function_name, "convert_to_grayscale") == 0)
    {
        return image_convert_to_grayscale(argc, argv);
    }

    // binarize_image
    if (strcmp(function_name, "binarize_image") == 0)
    {
        return image_binarize_image(argc, argv);
    }

    // convert_to_black_and_white
    if (strcmp(function_name, "convert_to_black_and_white") == 0)
    {
        return image_convert_to_black_and_white(argc, argv);
    }

    // median_filter_3x3
    if (strcmp(function_name, "median_filter_3x3") == 0)
    {
        return image_median_filter_3x3(argc, argv);
    }

    // rotate_image
    if (strcmp(function_name, "rotate_image") == 0)
    {
        return image_rotate_image(argc, argv);
    }

    // rotate_image_automatic
    if (strcmp(function_name, "rotate_image_automatic") == 0)
    {
        return image_rotate_image_automatic(argc, argv);
    }

    // slice_from
    if (strcmp(function_name, "slice_from") == 0)
    {
        return image_slice_from(argc, argv);
    }

    // slice_in_n
    if (strcmp(function_name, "slice_in_n") == 0)
    {
        return image_slice_in_n(argc, argv);
    }

    // crop
    if (strcmp(function_name, "crop") == 0)
    {
        return image_crop(argc, argv);
    }
}
#endif
