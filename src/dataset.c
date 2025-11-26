#include "../include/dataset.h"
#include "../include/image/image.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int char_to_index(char c) { return toupper(c) - 'A'; }

char index_to_char(int index) { return 'A' + index; }

double *one_hot(int label)
{
    double *v = calloc(26, sizeof(double));
    v[label] = 1.0;
    return v;
}

double *load_letter_image(const char *filename, int *input_size)
{
    GdkPixbuf *img = load_image((char *)filename);
    convert_to_grayscale(img);

    int w = gdk_pixbuf_get_width(img);
    int h = gdk_pixbuf_get_height(img);
    int stride = gdk_pixbuf_get_rowstride(img);
    int channels = gdk_pixbuf_get_n_channels(img);

    *input_size = w * h;

    double *vec = malloc(sizeof(double) * (*input_size));
    guchar *pixels = gdk_pixbuf_get_pixels(img);

    int k = 0;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            guchar *p = pixels + y * stride + x * channels;
            vec[k++] = p[0] / 255.0;
        }
    }

    g_object_unref(img);
    return vec;
}

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

            char file_path[300];
            sprintf(file_path, "%s/%s", letter_path, img->d_name);

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
