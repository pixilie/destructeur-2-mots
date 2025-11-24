#include "../include/image/image_helpers.h"
#include "../include/image/image_slice.h"
#include "../include/test_helpers.h"

#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <stdlib.h>

static void check_pixbuf_dimensions(GdkPixbuf *pixbuf, int expected_width,
                                    int expected_height)
{
    assert(gdk_pixbuf_get_width(pixbuf) == expected_width);
    assert(gdk_pixbuf_get_height(pixbuf) == expected_height);
}

int test_slice_in_n()
{
    print_test_subcategory("Slice in n Tests");

    const char *test_image1 = "square.jpg";

    GdkPixbuf *pixbuf1 =
        gdk_pixbuf_new_from_file(get_image_path(test_image1), NULL);
    if (!pixbuf1)
    {
        print_fail();
        printf("Could not load image '%s'\n", test_image1);
        return 0;
    }

    int n_slice = 3;
    int expected_size = gdk_pixbuf_get_width(pixbuf1) / n_slice;

    GdkPixbuf **tiles = slice_in_n(pixbuf1, n_slice);
    if (!tiles)
    {
        print_fail();
        printf("No tiles generated with slice_in_n\n");
        g_object_unref(pixbuf1);
        return 0;
    }

    for (int i = 0; i < n_slice * n_slice; i++)
    {
        check_pixbuf_dimensions(tiles[i], expected_size, expected_size);
    }

    for (int i = 0; i < n_slice * n_slice; i++)
    {
        g_object_unref(tiles[i]);
    }
    free(tiles);
    g_object_unref(pixbuf1);
    
    print_success();
    printf("slice_in_n() produced %d tiles of %dx%d\n",
           n_slice * n_slice, expected_size, expected_size);
    
    return 1;
}


int test_slice_from()
{
    print_test_subcategory("Slice From Tests");

    const char *test_image2 = "level_1_image_1.png";

    GdkPixbuf *pixbuf2 =
        gdk_pixbuf_new_from_file(get_image_path(test_image2), NULL);
    if (!pixbuf2)
    {
        print_fail();
        printf("Could not load image '%s'\n", test_image2);
        return 0;
    }

    int width = gdk_pixbuf_get_width(pixbuf2);
    int height = gdk_pixbuf_get_height(pixbuf2);
    int y_cut = height / 2;

    GdkPixbuf **horizontal = slice_from(pixbuf2, 0, y_cut, 1);
    if (!horizontal)
    {
        print_fail();
        printf("No tiles generated with horizontal cut of slice_from\n");
        g_object_unref(pixbuf2);
        return 0;
    }
    check_pixbuf_dimensions(horizontal[0], width, y_cut);
    check_pixbuf_dimensions(horizontal[1], width, height - y_cut);

    print_success();
    printf("Correctly cut image horizontally with slice_from\n");

    int x_cut = width / 2;
    GdkPixbuf **vertical = slice_from(pixbuf2, x_cut, 0, 0);
    if (!vertical)
    {
        print_fail();
        printf("No tiles generated with vertical cut of slice_from\n");
        g_object_unref(horizontal[0]);
        g_object_unref(horizontal[1]);
        free(horizontal);
        g_object_unref(pixbuf2);
        return 0;
    }
    check_pixbuf_dimensions(vertical[0], x_cut, height);
    check_pixbuf_dimensions(vertical[1], width - x_cut, height);

    g_object_unref(horizontal[0]);
    g_object_unref(horizontal[1]);
    g_object_unref(vertical[0]);
    g_object_unref(vertical[1]);
    free(horizontal);
    free(vertical);
    g_object_unref(pixbuf2);

    print_success();
    printf("Correctly cut image vertically with slice_from\n");

    return 1;
}

int test_crop()
{
    print_test_subcategory("Crop Tests");

    const char *test_image1 = "square.jpg";

    int x1 = 100;
    int y1 = 100;
    int x2 = 300;
    int y2 = 300;

    GdkPixbuf *pixbuf3 =
        gdk_pixbuf_new_from_file(get_image_path(test_image1), NULL);
    if (!pixbuf3)
    {
        print_fail();
        printf("Could not load image '%s'\n", test_image1);
        return 0;
    }

    GdkPixbuf *crop_result = crop(pixbuf3, x1, y1, x2, y2);
    if (!crop_result)
    {
        print_fail();
        printf("No image generated with crop\n");
        g_object_unref(pixbuf3);
        return 0;
    }

    check_pixbuf_dimensions(crop_result, x2 - x1, y2 - y1);

    g_object_unref(pixbuf3);
    g_object_unref(crop_result);

    print_success();
    printf("Produced a new slice of %dx%d with crop()\n", x2 - x1, y2 - y1);

    return 1;
}

int main(void)
{
    print_test_category("Image Slice Tests");

    int passed = 1;

    passed &= test_slice_in_n();
    passed &= test_slice_from();
    passed &= test_crop();

    if (passed)
    {
        print_all_tests_passed("Image Slice Tests");
    }
    else
    {
        print_some_tests_failed("Image Slice Tests");
    }

    return passed ? 0 : 1;
}
