#include "../include/image.h"
#include <assert.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int file_exists(const char *filename)
{
    struct stat buffer;
    return stat(filename, &buffer) == 0;
}

static void get_image_size(const char *path, int *width, int *height)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(get_image_path(path), &error);
    if (!pixbuf)
    {
        fprintf(stderr, "Error loading %s: %s\n", path,
                error ? error->message : "unknown");
        if (error)
            g_error_free(error);
        *width = *height = -1;
        return;
    }

    *width = gdk_pixbuf_get_width(pixbuf);
    *height = gdk_pixbuf_get_height(pixbuf);
    g_object_unref(pixbuf);
}

int main(void)
{
    gtk_init(NULL, NULL);
    printf("=== Running image slicing tests ===\n");

    const char *test_image = "level_1_image_1.png";

    if (!file_exists(get_image_path(test_image)))
    {
        fprintf(stderr, "Test image not found: %s\n", test_image);
        return 1;
    }

    int width = 0, height = 0;
    get_image_size(test_image, &width, &height);

    // ---------- Horizontal slice ----------
    printf("Testing horizontal slice...\n");
    int y_cut = height / 2;
    slice_from(test_image, 0, y_cut, 1);
    assert(file_exists("tests/results/top_img.png"));
    assert(file_exists("tests/results/bottom_img.png"));

    // ---------- Vertical slice ----------
    printf("Testing vertical slice...\n");
    int x_cut = width / 2;
    slice_from(test_image, x_cut, 0, 0);
    assert(file_exists("tests/results/left_img.png"));
    assert(file_exists("tests/results/right_img.png"));

    printf("All image slicing tests passed successfully.\n");
    return 0;
}
