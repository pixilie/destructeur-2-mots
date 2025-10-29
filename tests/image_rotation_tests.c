// tests/image_rotation_tests.c
#include "../include/image_helpers.h"
#include "../include/image_rotation.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

int test_rotate_90()
{
    printf("\n--- Rotate 90 Degrees Test ---\n");

    const char *path = get_image_path("level_1_image_1.png");
    if (!path)
    {
        printf("[FAIL] Failed to get image path\n");
        return 0;
    }

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (!pixbuf)
    {
        printf("[FAIL] Failed to load image\n");
        return 0;
    }

    GdkPixbuf *rotated = rotate_image(pixbuf, 90);
    if (!rotated)
    {
        printf("[FAIL] Rotation failed\n");
        g_object_unref(pixbuf);
        return 0;
    }

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int new_width = gdk_pixbuf_get_width(rotated);
    int new_height = gdk_pixbuf_get_height(rotated);

    // Basic dimension check
    int ok = (new_width >= height && new_height >= width);
    if (!ok)
    {
        printf("[FAIL] Rotated dimensions incorrect (original %dx%d, rotated %dx%d)\n",
               width, height, new_width, new_height);
    }
    else
    {
        printf("[SUCCESS] Rotated dimensions are correct\n");
    }

    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    guchar *top_left = pixels; // original top-left
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
        printf("[SUCCESS] Top-left pixel moved as expected\n");
    }
    else
    {
        printf("[FAIL] Top-left pixel did not move\n");
        ok = 0;
    }

    g_object_unref(pixbuf);
    g_object_unref(rotated);
    return ok;
}

int main()
{
    if (chdir("..") != 0)
    {
        perror("chdir failed");
        return 1;
    }

    int passed = 1;

    passed &= test_rotate_90();

    printf("\nImage Rotation Tests : %s\n",
           passed ? "All tests passed" : "Some tests did not pass");

    return passed ? 0 : 1;
}

