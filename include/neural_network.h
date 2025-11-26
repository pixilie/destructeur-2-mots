#pragma once

typedef struct
{
    int input_size;
    int hidden_size;
    int output_size;

    double *hidden_weights;
    double *output_weights;
    double *hidden_bias;
    double *output_bias;

    double *hidden;
    double *output;
} NeuralNetwork;

NeuralNetwork *create_network(int input_size, int hidden_size, int output_size);
NeuralNetwork *load_network(const char *filename);
void save_network(NeuralNetwork *nn, const char *filename);
void free_network(NeuralNetwork *nn);

double sigmoid(double x);
double sigmoid_derivative(double x);
double softmax(double *input, double *output, int size);
double rand_weight();

void forward(NeuralNetwork *nn, double *inputs);
void train(NeuralNetwork *nn, double **inputs, double **targets, int samples,
           double lr, int epochs);
