#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
    double **inputs;
    double **targets;
    int *labels;

    int samples;
    int input_size;
} Dataset;

Dataset load_dataset(const char *path);
void free_dataset(Dataset *d);

double *load_letter_image(const char *filename, int *input_size);
double *one_hot(int label);
int char_to_index(char c);
char index_to_char(int index);
