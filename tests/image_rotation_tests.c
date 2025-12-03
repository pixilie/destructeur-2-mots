#include "../include/image/image_helpers.h"
#include "../include/image/image_processing.h"
#include "../include/image/image_rotation.h"
#include "../include/test_helpers.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <stdio.h>

int is_best_angle_correct(double expected_angle, double actual_angle,
                          double tolerance)
{
    if (fabs(expected_angle - actual_angle) < tolerance)
    {
        return 1;
    }
    return 0;
}

int find_best_rotation_angle(char *filename, char *image_print_name,
                             double expected_angle, double tolerance)
{
    GdkPixbuf *pixbuf = load_image(filename);
    if (!pixbuf)
    {
        return 0;
    }

    convert_to_black_and_white(pixbuf);
    double actual_best_rotation_angle = detect_best_angle(pixbuf);
    int result = is_best_angle_correct(expected_angle,
                                       actual_best_rotation_angle, tolerance);

    if (result)
    {
        print_success();
        printf("Image : %s. Expected best rotation angle %.2f°, got: %.2f°\n",
               image_print_name, expected_angle, actual_best_rotation_angle);
    }
    else
    {
        print_fail();
        printf("Image : %s. Expected best rotation angle %.2f°, got: %.2f°\n",
               image_print_name, expected_angle, actual_best_rotation_angle);
    }

    return result;
}

int test_detect_best_angle()
{
    print_test_subcategory("Detect Best Angle Tests");

    int res = find_best_rotation_angle("level_1_image_1.png", "Level 1 Image 1",
                                       0, 1);
    res &= find_best_rotation_angle("level_2_image_1.png", "Level 2 Image 1",
                                    -25, 1);
    res &= find_best_rotation_angle("level_2_image_2.png", "Level 2 Image 2", 5,
                                    1);

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
