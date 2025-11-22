#include "../include/image/image_helpers.h"
#include "../include/image/image_processing.h"
#include "../include/test_helpers.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>

int test_grayscale()
{
    print_test_subcategory("Grayscale Tests");

    const char *path = get_image_path("level_1_image_1.png");
    if (!path)
    {
        print_fail();
        printf("Failed to load image\n");
        return 0;
    }
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (!pixbuf)
    {
        print_fail();
        printf("Failed to load image\n");
        return 0;
    }

    convert_to_grayscale(pixbuf);
    if (!pixbuf)
    {
        print_fail();
        printf("Grayscale failed\n");
        g_object_unref(pixbuf);
        return 0;
    }

    // Check that R = G = B for each pixel (levels of gray)
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    int ok = 1;
    for (int y = 0; y < height && ok; y++)
    {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *p = row + x * n_channels;
            if (!(p[0] == p[1] && p[1] == p[2]))
            {
                ok = 0;
                break;
            }
        }
    }

    if (ok)
    {
        print_success();
        printf("Successfully converted image %s to grayscale\n", path);
    }
    else
    {
        print_fail();
        printf("Grayscale failed\n");
    }
    g_object_unref(pixbuf);
    return ok;
}

int test_black_and_white()
{
    print_test_subcategory("Binarize Tests");

    const char *path = get_image_path("level_1_image_1.png");
    if (!path)
    {
        print_fail();
        printf("Failed to load image\n");
        return 0;
    }
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (!pixbuf)
    {
        print_fail();
        printf("Failed to load image\n");
        return 0;
    }

    convert_to_grayscale(pixbuf);
    if (!pixbuf)
    {
        print_fail();
        printf("Grayscale failed\n");
        g_object_unref(pixbuf);
        return 0;
    }

    int threshold = convert_to_black_and_white(pixbuf);
    if (!pixbuf)
    {
        print_fail();
        printf("Binarize failed");
        g_object_unref(pixbuf);
        return 0;
    }

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    int ok = 1;
    for (int y = 0; y < height && ok; y++)
    {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < width; x++)
        {
            guchar *p = row + x * n_channels;
            if (!((p[0] == 0 && p[1] == 0 && p[2] == 0) ||
                  (p[0] == 255 && p[1] == 255 && p[2] == 255)))
            {
                ok = 0;
                break;
            }
        }
    }

    if (ok)
    {
        print_success();
        printf("Successfully converted image to black and white with"
               " threshold %i\n", threshold);
    }
    else
    {
        print_fail();
        printf("Binarize failed\n");
    }
    g_object_unref(pixbuf);
    return ok;
}

int main()
{
    if (chdir("..") != 0)
    {
        perror("chdir failed");
        return 1;
    }

    print_test_category("Image Processing Tests");

    int passed = 1;

    passed &= test_grayscale();
    passed &= test_black_and_white();

    if (passed)
    {
        print_all_tests_passed("Image Processing Tests");
    }
    else
    {
        print_some_tests_failed("Image Processing Tests");
    }
    return passed ? 0 : 1;
}
