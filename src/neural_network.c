#include "../include/neural_network.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Compute the sigmoid activation function.
 *
 * Parameters:
 *  - x: input value
 *
 * Returns:
 *  - sigmoid(x) in the interval (0, 1)
 */
double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

/**
 * Compute the derivative of the sigmoid function.
 *
 * Parameters:
 *  - x: the sigmoid output value s = sigmoid(z)
 *
 * Returns:
 *  - derivative ds/dz = s * (1 - s)
 */
double sigmoid_derivative(double x) { return x * (1.0 - x); }

/**
 * Generate a random weight in the range [-1.0, 1.0].
 *
 * Returns:
 *  - random double uniformly distributed between -1 and 1
 */
double rand_weight() { return ((double)rand() / RAND_MAX) * 2.0 - 1.0; }

/**
 * Allocate and initialize a neural network with one hidden layer.
 *
 * Parameters:
 *  - input_size : number of input neurons
 *  - hidden_size: number of neurons in the hidden layer
 *  - output_size: number of output neurons
 *
 * Returns:
 *  - pointer to an allocated NeuralNetwork with randomly initialized weights
 * and biases
 */
NeuralNetwork *create_network(int input_size, int hidden_size, int output_size)
{
    NeuralNetwork *nn = malloc(sizeof(NeuralNetwork));
    nn->input_size = input_size;
    nn->hidden_size = hidden_size;
    nn->output_size = output_size;

    nn->hidden_weights = malloc(sizeof(double) * input_size * hidden_size);
    nn->output_weights = malloc(sizeof(double) * hidden_size * output_size);
    nn->hidden_bias = malloc(sizeof(double) * hidden_size);
    nn->output_bias = malloc(sizeof(double) * output_size);
    nn->hidden = malloc(sizeof(double) * hidden_size);
    nn->output = malloc(sizeof(double) * output_size);

    srand(time(NULL));
    for (int i = 0; i < input_size * hidden_size; i++)
        nn->hidden_weights[i] = rand_weight();
    for (int i = 0; i < hidden_size * output_size; i++)
        nn->output_weights[i] = rand_weight();
    for (int i = 0; i < hidden_size; i++)
        nn->hidden_bias[i] = rand_weight();
    for (int i = 0; i < output_size; i++)
        nn->output_bias[i] = rand_weight();

    return nn;
}

/**
 * Free all memory allocated for the neural network.
 *
 * Parameters:
 *  - nn: pointer to the NeuralNetwork to free (must not be used after this
 * call)
 */
void free_network(NeuralNetwork *nn)
{
    free(nn->hidden_weights);
    free(nn->output_weights);
    free(nn->hidden_bias);
    free(nn->output_bias);
    free(nn->hidden);
    free(nn->output);
    free(nn);
}

/**
 * Perform a forward pass (inference) through the network.
 *
 * Parameters:
 *  - nn    : pointer to an initialized NeuralNetwork
 *  - inputs: array of input values (length must be nn->input_size)
 */
void forward(NeuralNetwork *nn, double *inputs)
{
    // Hidden layer
    for (int i = 0; i < nn->hidden_size; i++)
    {
        double sum = nn->hidden_bias[i];
        for (int j = 0; j < nn->input_size; j++)
            sum += inputs[j] * nn->hidden_weights[j * nn->hidden_size + i];
        nn->hidden[i] = sigmoid(sum);
    }

    // Output layer
    for (int i = 0; i < nn->output_size; i++)
    {
        double sum = nn->output_bias[i];
        for (int j = 0; j < nn->hidden_size; j++)
            sum += nn->hidden[j] * nn->output_weights[j * nn->output_size + i];
        nn->output[i] = sigmoid(sum);
    }
}

/**
 * Train the network on a small fixed dataset (typically XOR).
 *
 * Parameters:
 *  - nn           : pointer to the NeuralNetwork to train
 *  - inputs       : 4x2 array containing 4 training examples (each of size 2)
 *  - targets      : array of 4 target values (one target per example)
 *  - learning_rate: learning rate for weight updates
 *  - epochs       : number of training iterations (epochs)
 */
void train(NeuralNetwork *nn, double inputs[4][2], double targets[4],
           double learning_rate, int epochs)
{
    for (int epoch = 0; epoch < epochs; epoch++)
    {
        double total_error = 0.0;
        for (int s = 0; s < 4; s++)
        {
            double *x = inputs[s];
            double y = targets[s];

            // Forward pass
            forward(nn, x);

            double output = nn->output[0];
            double error = y - output;
            total_error += error * error;

            // Backpropagation
            double output_delta = error * sigmoid_derivative(output);

            double *hidden_deltas = malloc(sizeof(double) * nn->hidden_size);
            for (int i = 0; i < nn->hidden_size; i++)
            {
                double h = nn->hidden[i];
                double w = nn->output_weights[i * nn->output_size + 0];
                hidden_deltas[i] = sigmoid_derivative(h) * output_delta * w;
            }

            // Update output weight
            for (int i = 0; i < nn->hidden_size; i++)
                nn->output_weights[i] +=
                    learning_rate * output_delta * nn->hidden[i];
            nn->output_bias[0] += learning_rate * output_delta;

            // Update hidden weight
            for (int i = 0; i < nn->input_size; i++)
            {
                for (int j = 0; j < nn->hidden_size; j++)
                {
                    nn->hidden_weights[i * nn->hidden_size + j] +=
                        learning_rate * hidden_deltas[j] * x[i];
                }
            }
            for (int j = 0; j < nn->hidden_size; j++)
                nn->hidden_bias[j] += learning_rate * hidden_deltas[j];

            free(hidden_deltas);
        }

        if (epoch % 1000 == 0)
        {
            printf("Epoch %d - Error: %.6f\n", epoch, total_error);
        }
    }
}
