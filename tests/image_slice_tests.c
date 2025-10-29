#include "../include/image_helpers.h"
#include "../include/image_slice.h"
#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

static void check_pixbuf_dimensions(GdkPixbuf *pixbuf, int expected_width, int expected_height)
{
    assert(gdk_pixbuf_get_width(pixbuf) == expected_width);
    assert(gdk_pixbuf_get_height(pixbuf) == expected_height);
}

int main(void)
{
    gtk_init(NULL, NULL);

    const char *test_image1 = "square.jpg";
    const char *test_image2 = "level_1_image_1.png";

    printf("\n--- TEST: slice_in_n() ---\n");

    GError *error = NULL;
    GdkPixbuf *pixbuf1 = gdk_pixbuf_new_from_file(get_image_path(test_image1), &error);
    if (!pixbuf1)
    {
        fprintf(stderr, "FAIL: Could not load image '%s': %s\n", test_image1,
                error ? error->message : "unknown error");
        if (error) g_error_free(error);
        return 1;
    }

    int n_slice = 3;
    int expected_size = gdk_pixbuf_get_width(pixbuf1) / n_slice;

    GdkPixbuf **tiles = slice_in_n(pixbuf1, n_slice);
    if (!tiles)
    {
        fprintf(stderr, "FAIL: slice_in_n returned NULL\n");
        g_object_unref(pixbuf1);
        return 1;
    }
    for (int i = 0; i < n_slice * n_slice; i++)
        check_pixbuf_dimensions(tiles[i], expected_size, expected_size);

    printf("PASS: slice_in_n() produced %d tiles of %dx%d\n",
           n_slice * n_slice, expected_size, expected_size);

    for (int i = 0; i < n_slice * n_slice; i++)
        g_object_unref(tiles[i]);
    free(tiles);
    g_object_unref(pixbuf1);

    printf("\n--- TEST: slice_from() ---\n");

    GdkPixbuf *pixbuf2 = gdk_pixbuf_new_from_file(get_image_path(test_image2), &error);
    if (!pixbuf2)
    {
        fprintf(stderr, "FAIL: Could not load image '%s': %s\n", test_image2,
                error ? error->message : "unknown error");
        if (error) g_error_free(error);
        return 1;
    }

    int width = gdk_pixbuf_get_width(pixbuf2);
    int height = gdk_pixbuf_get_height(pixbuf2);

    int y_cut = height / 2;
    GdkPixbuf **horizontal = slice_from(pixbuf2, 0, y_cut, 1);
    if (!horizontal)
    {
        fprintf(stderr, "FAIL: slice_from horizontal returned NULL\n");
        g_object_unref(pixbuf2);
        return 1;
    }
    check_pixbuf_dimensions(horizontal[0], width, y_cut);
    check_pixbuf_dimensions(horizontal[1], width, height - y_cut);

    int x_cut = width / 2;
    GdkPixbuf **vertical = slice_from(pixbuf2, x_cut, 0, 0);
    if (!vertical)
    {
        fprintf(stderr, "FAIL: slice_from vertical returned NULL\n");
        g_object_unref(horizontal[0]);
        g_object_unref(horizontal[1]);
        g_object_unref(pixbuf2);
        return 1;
    }
    check_pixbuf_dimensions(vertical[0], x_cut, height);
    check_pixbuf_dimensions(vertical[1], width - x_cut, height);

    printf("PASS: slice_from() horizontal and vertical slices correct\n");

    g_object_unref(horizontal[0]);
    g_object_unref(horizontal[1]);
    free(horizontal);

    g_object_unref(vertical[0]);
    g_object_unref(vertical[1]);
    free(vertical);

    g_object_unref(pixbuf2);

    printf("All pixbuf slicing tests passed successfully.\n");
    return 0;
}
