#include "../include/neural_network.h"
#include "image/image_helpers.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * ReLU (Rectified Linear Unit) activation function.
 * Used for the hidden layers to introduce non-linearity.
 *
 * Parameters:
 * - x : input value
 *
 * Returns:
 * - x if x > 0, else 0
 */
double relu(double x)
{
    if (x > 0)
    {
        return x;
    }
    else
    {
        return 0;
    }
}

/**
 * Derivative of the ReLU activation function.
 * Used during backpropagation to calculate gradients.
 *
 * Parameters:
 * - x : the raw input (pre-activation sum) stored during forward pass
 *
 * Returns:
 * - 1 if x > 0, else 0
 */
double relu_derivative(double x)
{
    if (x > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Softmax activation function (stable version).
 * Converts raw logits into a probability distribution.
 * Subtracts the max value to improve numerical stability against large
 * exponentials.
 *
 * Parameters:
 * - input  : array of raw logits (size = size)
 * - output : array that will receive calculated probabilities (size = size)
 * - size   : number of elements in the layer
 *
 * Returns:
 * - normalization factor (sum of exponentials)
 */
double softmax(double *input, double *output, int size)
{
    double max = input[0];
    for (int i = 1; i < size; i++)
    {
        if (input[i] > max)
        {
            max = input[i];
        }
    }

    double sum = 0.0;

    for (int i = 0; i < size; i++)
    {
        output[i] = exp(input[i] - max);
        sum += output[i];
    }

    for (int i = 0; i < size; i++)
    {
        output[i] /= sum;
    }

    return sum;
}

/**
 * Initialize weights using He (Kaiming) Initialization.
 * Best suited for layers using ReLU activation.
 * Formula: Uniform distribution between [-limit, limit]
 * where limit = sqrt(6 / fan_in).
 *
 * Parameters:
 * - weights : pointer to the weight array to initialize
 * - size    : total number of weights in the array
 * - fan_in  : number of input neurons feeding into this layer
 */
void init_weights_he(double *weights, int size, int fan_in)
{
    double limit = sqrt(6.0 / (double)fan_in);

    for (int i = 0; i < size; i++)
    {
        double random_val = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        weights[i] = random_val * limit;
    }
}

/**
 * Allocate and initialize a new neural network structure.
 * Uses He initialization for weights and sets biases to 0.0.
 *
 * Parameters:
 * - input_size  : number of input neurons (e.g., 784 for 28x28 images)
 * - hidden_size : number of neurons in the hidden layer
 * - output_size : number of output neurons (e.g., 26 for A-Z)
 *
 * Returns:
 * - pointer to the allocated NeuralNetwork struct
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

    nn->hidden_raw = malloc(sizeof(double) * hidden_size);
    nn->hidden = malloc(sizeof(double) * hidden_size);
    nn->output = malloc(sizeof(double) * output_size);

    srand(time(NULL));
    init_weights_he(nn->hidden_weights, input_size * hidden_size, input_size);
    init_weights_he(nn->output_weights, hidden_size * output_size, hidden_size);

    for (int i = 0; i < hidden_size; i++)
    {
        nn->hidden_bias[i] = 0.0;
    }

    for (int i = 0; i < output_size; i++)
    {
        nn->output_bias[i] = 0.0;
    }

    return nn;
}

/**
 * Free all memory allocated for a neural network.
 *
 * Parameters:
 * - nn : pointer to the NeuralNetwork to destroy
 */
void free_network(NeuralNetwork *nn)
{
    free(nn->hidden_weights);
    free(nn->output_weights);
    free(nn->hidden_bias);
    free(nn->output_bias);
    free(nn->hidden_raw);
    free(nn->hidden);
    free(nn->output);
    free(nn);
}

/**
 * Save the neural network model to a binary file.
 * Saves topology (sizes), weights, and biases.
 *
 * Parameters:
 * - nn       : pointer to the neural network
 * - filename : path where the file will be saved
 */
void save_network(NeuralNetwork *nn, const char *filename)
{
    FILE *f = fopen(filename, "wb");

    if (!f)
    {
        printf("Error: cannot open %s\n", filename);
        return;
    }

    fwrite(&nn->input_size, sizeof(int), 1, f);
    fwrite(&nn->hidden_size, sizeof(int), 1, f);
    fwrite(&nn->output_size, sizeof(int), 1, f);

    fwrite(nn->hidden_weights, sizeof(double), nn->input_size * nn->hidden_size,
           f);
    fwrite(nn->output_weights, sizeof(double),
           nn->hidden_size * nn->output_size, f);

    fwrite(nn->hidden_bias, sizeof(double), nn->hidden_size, f);
    fwrite(nn->output_bias, sizeof(double), nn->output_size, f);

    fclose(f);
}

/**
 * Load a neural network from a binary file.
 *
 * Parameters:
 * - filename : path to the saved model file
 *
 * Returns:
 * - pointer to the loaded NeuralNetwork, or NULL on error
 */
NeuralNetwork *load_network(const char *filename)
{
    FILE *f = fopen(filename, "rb");

    if (!f)
    {
        printf("Error: cannot open %s\n", filename);
        return NULL;
    }

    int input, hidden, output;

    fread(&input, sizeof(int), 1, f);
    fread(&hidden, sizeof(int), 1, f);
    fread(&output, sizeof(int), 1, f);

    NeuralNetwork *nn = create_network(input, hidden, output);

    fread(nn->hidden_weights, sizeof(double), input * hidden, f);
    fread(nn->output_weights, sizeof(double), hidden * output, f);
    fread(nn->hidden_bias, sizeof(double), hidden, f);
    fread(nn->output_bias, sizeof(double), output, f);

    fclose(f);
    return nn;
}

/**
 * Perform a forward pass through the network.
 * Computes hidden layer (Linear -> ReLU) and output layer (Linear -> Softmax).
 * Results are stored in nn->hidden and nn->output.
 *
 * Parameters:
 * - nn     : pointer to the neural network
 * - inputs : input array of size nn->input_size
 */
void forward(NeuralNetwork *nn, double *inputs)
{
    // Hidden Layer Calculation
    for (int i = 0; i < nn->hidden_size; i++)
    {
        double sum = nn->hidden_bias[i];

        for (int j = 0; j < nn->input_size; j++)
        {
            sum += inputs[j] * nn->hidden_weights[j * nn->hidden_size + i];
        }

        nn->hidden_raw[i] = sum;
        nn->hidden[i] = relu(sum);
    }

    // Output Layer Calculation
    double *logits = malloc(sizeof(double) * nn->output_size);

    for (int i = 0; i < nn->output_size; i++)
    {
        double sum = nn->output_bias[i];

        for (int h = 0; h < nn->hidden_size; h++)
        {
            sum += nn->hidden[h] * nn->output_weights[h * nn->output_size + i];
        }

        logits[i] = sum;
    }

    // Output Activation
    softmax(logits, nn->output, nn->output_size);

    free(logits);
}

/**
 * Train the network using Stochastic Gradient Descent (SGD).
 * Performs forward pass, calculates Cross-Entropy loss, performs
 * backpropagation, and updates weights/biases.
 *
 * Parameters:
 * - nn      : pointer to neural network
 * - inputs  : array of input vectors (training data)
 * - targets : array of target vectors (one-hot encoded)
 * - samples : number of training samples
 * - lr      : learning rate (e.g., 0.01)
 * - epochs  : number of times to iterate over the entire dataset
 */
void train(NeuralNetwork *nn, double **inputs, double **targets, int samples,
           double lr, int epochs)
{
    for (int epoch = 0; epoch < epochs; epoch++)
    {
        double total_loss = 0.0;

        for (int s = 0; s < samples; s++)
        {
            forward(nn, inputs[s]);

            double *output_deltas = malloc(sizeof(double) * nn->output_size);
            double *hidden_deltas = malloc(sizeof(double) * nn->hidden_size);

            // Calculate Loss (Cross-Entropy)
            for (int o = 0; o < nn->output_size; o++)
            {
                if (targets[s][o] == 1.0)
                {
                    total_loss += -log(nn->output[o] + 1e-12);
                }
            }

            // Output Layer Deltas (Softmax + Cross-Entropy Derivative = O - T)
            for (int o = 0; o < nn->output_size; o++)
            {
                output_deltas[o] = nn->output[o] - targets[s][o];
            }

            // Hidden Layer Deltas (Backpropagation)
            for (int h = 0; h < nn->hidden_size; h++)
            {
                double sum = 0.0;
                for (int o = 0; o < nn->output_size; o++)
                {
                    sum += output_deltas[o] *
                           nn->output_weights[h * nn->output_size + o];
                }

                hidden_deltas[h] = relu_derivative(nn->hidden_raw[h]) * sum;
            }

            // Update Output Weights and Biases
            for (int h = 0; h < nn->hidden_size; h++)
            {
                for (int o = 0; o < nn->output_size; o++)
                {
                    nn->output_weights[h * nn->output_size + o] -=
                        lr * output_deltas[o] * nn->hidden[h];
                }
            }
            for (int o = 0; o < nn->output_size; o++)
            {
                nn->output_bias[o] -= lr * output_deltas[o];
            }

            // Update Hidden Weights and Biases
            for (int i = 0; i < nn->input_size; i++)
            {
                for (int h = 0; h < nn->hidden_size; h++)
                {
                    nn->hidden_weights[i * nn->hidden_size + h] -=
                        lr * hidden_deltas[h] * inputs[s][i];
                }
            }
            for (int h = 0; h < nn->hidden_size; h++)
            {
                nn->hidden_bias[h] -= lr * hidden_deltas[h];
            }

            free(output_deltas);
            free(hidden_deltas);
        }

        // if (epoch % 100 == 0)
        // {
        //     printf("Epoch %d - Loss: %.6f\n", epoch, total_loss);
        // }
    }
}

/**
 * Predict a character (A-Z) from an image.
 * Wraps image preprocessing and network inference.
 *
 * Parameters:
 * - nn     : pointer to the trained neural network
 * - pixbuf : input image (GdkPixbuf)
 *
 * Returns:
 * - predicted character (uppercase char)
 */
char predict_letter(NeuralNetwork *nn, GdkPixbuf *pixbuf)
{
    GdkPixbuf *scaled_pixbuf = scale_pixbuf_to_28x28(pixbuf);

    double input[28 * 28];
    pixbuf_to_input_vector(scaled_pixbuf, input);

    forward(nn, input);
    g_object_unref(scaled_pixbuf);

    int best = 0;
    double best_val = nn->output[0];

    for (int i = 1; i < nn->output_size; i++)
    {
        if (nn->output[i] > best_val)
        {
            best_val = nn->output[i];
            best = i;
        }
    }

    return (char)('A' + best);
}
