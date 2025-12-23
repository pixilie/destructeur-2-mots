#include "../include/dataset.h"
#include "../include/image_processing/image_processing.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * char_to_index:
 * Convert an ASCII letter to an index in [0,25].
 * Parameters:
 *  - c: letter (A-Z or a-z)
 * Returns:
 *  - index for the letter (A -> 0, B -> 1, ..., Z -> 25)
 */
int char_to_index(char c) { return toupper(c) - 'A'; }

/*
 * index_to_char:
 * Convert an index in [0,25] back to an uppercase ASCII letter.
 * Parameters:
 *  - index: integer between 0 and 25
 * Returns:
 *  - corresponding uppercase character ('A' + index)
 */
char index_to_char(int index) { return 'A' + index; }

/*
 * one_hot:
 * Create a one-hot vector of length 26 for a given class label.
 * Parameters:
 *  - label: class index in [0,25]
 * Returns:
 *  - pointer to malloc'd double[26] with 1.0 at position label and 0.0
 * elsewhere. Caller is responsible for freeing the returned array.
 */
double *one_hot(int label)
{
    double *v = calloc(26, sizeof(double));
    v[label] = 1.0;
    return v;
}

/*
 * load_letter_image:
 * Load an image file, convert to grayscale, scale to 28x28 and normalize.
 * Parameters:
 *  - filename: path to image file
 *  - input_size: out parameter receiving number of pixels (width * height)
 * Returns:
 *  - malloc'd double array of normalized pixels (values in [0.0,1.0]),
 *    or NULL on failure. Caller must free the returned array.
 */
double *load_letter_image(const char *filename, int *input_size)
{
    GdkPixbuf *img = load_image((char *)filename);

    if (!img)
    {
        return NULL;
    }

    convert_to_grayscale(img);
    GdkPixbuf *scaled_img = scale_pixbuf_to_28x28(img);
    g_object_unref(img);
    img = scaled_img;

    int w = gdk_pixbuf_get_width(img);
    int h = gdk_pixbuf_get_height(img);
    int rowstride = gdk_pixbuf_get_rowstride(img);
    int n_channels = gdk_pixbuf_get_n_channels(img);
    guchar *pixels = gdk_pixbuf_get_pixels(img);

    *input_size = w * h;
    double *vec = malloc(sizeof(double) * (*input_size));

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            guchar p = pixels[y * rowstride + x * n_channels];
            vec[y * w + x] = (255.0 - (double)p) / 255.0;
        }
    }

    g_object_unref(img);
    return vec;
}

/*
 * load_dataset:
 * Load a dataset stored in a directory tree where each subdirectory is named
 * by a letter and contains .png images for that class.
 *
 * Expected layout:
 *   path/A/image1.png
 *   path/A/image2.png
 *   path/B/image1.png
 *   ...
 *
 * Parameters:
 *  - path: root directory of the dataset
 *
 * Returns:
 *  - Dataset structure with arrays of inputs, targets, labels and counts.
 *    The inputs and targets arrays contain malloc'd pointers that the caller
 *    must free using free_dataset().
 */
Dataset load_dataset(const char *path)
{
    Dataset d = {0};
    d.samples = 0;

    int samples_capacity = 1000;

    d.inputs = malloc(samples_capacity * sizeof(double *));
    d.targets = malloc(samples_capacity * sizeof(double *));
    d.labels = malloc(samples_capacity * sizeof(int));

    DIR *dir = opendir(path);
    struct dirent *entry;

    int input_size_set = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_DIR)
        {
            continue;
        }

        char letter = entry->d_name[0];
        if (!isalpha(letter))
        {
            continue;
        }

        int label = char_to_index(letter);

        char letter_path[256];
        sprintf(letter_path, "%s/%c", path, letter);

        DIR *sub = opendir(letter_path);
        struct dirent *img;

        while ((img = readdir(sub)) != NULL)
        {
            if (strstr(img->d_name, ".png") == NULL)
            {
                continue;
            }

            char file_path[4096];
            snprintf(file_path, sizeof(file_path), "%s/%s", letter_path,
                     img->d_name);

            int size;
            double *vec = load_letter_image(file_path, &size);

            if (!input_size_set)
            {
                d.input_size = size;
                input_size_set = 1;
            }

            if (d.samples >= samples_capacity)
            {
                samples_capacity *= 2;

                d.inputs =
                    realloc(d.inputs, samples_capacity * sizeof(double *));
                d.targets =
                    realloc(d.targets, samples_capacity * sizeof(double *));
                d.labels = realloc(d.labels, samples_capacity * sizeof(int));
            }

            d.inputs[d.samples] = vec;
            d.targets[d.samples] = one_hot(label);
            d.labels[d.samples] = label;

            d.samples++;
        }

        closedir(sub);
    }

    closedir(dir);
    return d;
}

/*
 * free_dataset:
 * Free memory allocated for a Dataset returned by load_dataset.
 * Parameters:
 *  - d: pointer to Dataset to free
 */
void free_dataset(Dataset *d)
{
    for (int i = 0; i < d->samples; i++)
    {
        free(d->inputs[i]);
        free(d->targets[i]);
    }

    free(d->inputs);
    free(d->targets);
    free(d->labels);
}
