#include "../include/image_helpers.h"
#include "../include/image_processing.h"
#include "../include/image_rotation.h"
#include "../include/test_helpers.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <stdio.h>

int is_best_angle_correct(double expected_angle, double actual_angle, double tolerance)
{
    if (fabs(expected_angle - actual_angle) < tolerance)
    {
        return 1;
    }
    return 0;
}

int find_best_rotation_angle(char *filename, char *image_print_name, double expected_angle, double tolerance)
{
    GdkPixbuf *pixbuf = load_image(filename);
    if (!pixbuf)
    {
        return 0;
    }
    
    convert_to_black_and_white(pixbuf);
    double actual_best_rotation_angle = detect_best_angle(pixbuf);
    int result = is_best_angle_correct(expected_angle, actual_best_rotation_angle, tolerance);
    
    if (result)
    {
        print_success();
        printf("Image : %s. Expected best rotation angle %f, got: %f\n", image_print_name, expected_angle, actual_best_rotation_angle);
    }
    else
    {
        print_fail();
        printf("Image : %s. Expected best rotation angle %f, got: %f\n", image_print_name, expected_angle, actual_best_rotation_angle);
    }
    
    return result;
}

int test_rotate_90()
{
    print_test_subcategory("Rotate 90 Degrees Tests");

    GdkPixbuf *pixbuf = load_image("level_1_image_1.png");
    if (!pixbuf)
    {
        print_fail();
        printf("Failed to load image\n");
        return 0;
    }

    GdkPixbuf *rotated = rotate_image(pixbuf, 90);
    if (!rotated)
    {
        print_fail();
        printf("Rotation failed\n");
        g_object_unref(pixbuf);
        return 0;
    }

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int new_width = gdk_pixbuf_get_width(rotated);
    int new_height = gdk_pixbuf_get_height(rotated);
    
    // Basic dimension check
    int is_rotation_correct = (new_width >= height && new_height >= width);
    if (!is_rotation_correct)
    {
        print_fail();
        printf("Rotated dimensions incorrect (original %dx%d, rotated %dx%d)\n",
               width, height, new_width, new_height);
    }
    else
    {
        print_success();
        printf("Rotated dimensions are correct\n");
    }

    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    guchar *top_left = pixels;
    guchar *rot_pixels = gdk_pixbuf_get_pixels(rotated);
    int rot_rowstride = gdk_pixbuf_get_rowstride(rotated);

    guchar *new_top_left = rot_pixels + 0 * rot_rowstride + 0 * n_channels;
    int pixel_same = 1;
    for (int c = 0; c < n_channels; c++)
    {
        if (top_left[c] != new_top_left[c])
        {
            pixel_same = 0;
            break;
        }
    }

    if (!pixel_same)
    {
        print_success();
        printf("Top-left pixel moved as expected\n");
    }
    else
    {
        print_fail();
        printf("Top-left pixel did not move\n");
        is_rotation_correct = 0;
    }

    g_object_unref(pixbuf);
    g_object_unref(rotated);
    return is_rotation_correct;
}

int test_detect_best_angle()
{
    print_test_subcategory("Detect Best Angle Tests");
    
    int res = find_best_rotation_angle("level_1_image_1.png", "Level 1 Image 1", 0, 1);
    res &= find_best_rotation_angle("level_2_image_1.png", "Level 2 Image 1", -25, 1);
    res &= find_best_rotation_angle("level_2_image_2.png", "Level 2 Image 2", 5, 1);
    
    return res;    
}

int main()
{
    if (chdir("..") != 0)
    {
        perror("chdir failed");
        return 1;
    }

    print_test_category("Image Rotation Tests");

    int passed = 1;

    passed &= test_rotate_90();
    passed &= test_detect_best_angle();

    if (passed)
    {
        print_all_tests_passed("Image Rotation Tests");
    }
    else
    {
        print_some_tests_failed("Image Rotation Tests");
    }
    return passed ? 0 : 1;
}

