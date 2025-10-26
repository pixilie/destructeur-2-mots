#pragma once

typedef struct {
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
void free_network(NeuralNetwork *nn);
double sigmoid(double x);
double sigmoid_derivative(double x);
double rand_weight();
void forward(NeuralNetwork *nn, double *inputs);
void train(NeuralNetwork *nn, double inputs[4][2], double targets[4], double learning_rate, int epochs);
