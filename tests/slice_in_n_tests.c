#include "../include/image.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int file_exists(const char *filename)
{
    struct stat buffer;
    return stat(filename, &buffer) == 0;
}

static int count_generated_slices(int n_slice)
{
    int count = 0;
    char filename[256];
    for (int i = 0; i < n_slice; i++)
    {
        for (int j = 0; j < n_slice; j++)
        {
            snprintf(filename, sizeof(filename), "slice_%d_%d.png", i, j);
            if (file_exists(filename))
                count++;
        }
    }
    return count;
}

int main(void)
{
    gtk_init(NULL, NULL);

    const char *test_image = "../assets/level_1_image_1.png";
    const int n_slice = 3;
    const int expected_slices = n_slice * n_slice;

    printf("=== TEST: slice_in_n() ===\n");

    if (!file_exists(test_image))
    {
        fprintf(stderr, "FAIL: Test image not found at path '%s'.\n",
                test_image);
        return 1;
    }

    slice_in_n(test_image, n_slice);

    int found_slices = count_generated_slices(n_slice);

    if (found_slices != expected_slices)
    {
        fprintf(stderr, "FAIL: Expected %d slices, but found %d.\n",
                expected_slices, found_slices);
        return 1;
    }

    printf("PASS: All %d slice files created successfully.\n", found_slices);

    printf("Cleanup complete. Test passed.\n");
    return 0;
}
