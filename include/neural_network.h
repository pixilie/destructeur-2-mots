#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
    int input_size;
    int hidden_size;
    int output_size;

    double *hidden_weights;
    double *output_weights;
    double *hidden_bias;
    double *output_bias;

    double *hidden_raw;
    double *hidden;
    double *output;
} NeuralNetwork;

NeuralNetwork *create_network(int input_size, int hidden_size, int output_size);
NeuralNetwork *load_network(const char *filename);
void save_network(NeuralNetwork *nn, const char *filename);
void free_network(NeuralNetwork *nn);

double relu(double x);
double relu_derivative(double x);
double softmax(double *input, double *output, int size);
void init_weights_he(double *weights, int size, int fan_in);

void forward(NeuralNetwork *nn, double *inputs);
void train(NeuralNetwork *nn, double **inputs, double **targets, int samples,
           double lr, int epochs);
char predict_letter(NeuralNetwork *nn, GdkPixbuf *pixbuf);
