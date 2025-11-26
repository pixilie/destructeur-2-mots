#include "../include/neural_network.h"
#include "image/image_helpers.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LETTERS 26

/**
 * ReLU activation function.
 *
 * Parameters:
 *  - x : input value
 *
 * Returns:
 *  - max(0, x)
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
 * Derivative of the ReLU activation.
 *
 * Parameters:
 *  - x : the activated value
 *
 * Returns:
 *  - 1 if x > 0, else 0
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
 * Softmax activation (stable version).
 *
 * Parameters:
 *  - input  : array of raw logits
 *  - output : array that will receive softmax probabilities
 *  - size   : number of elements
 *
 * Returns:
 *  - normalization factor (sum of exponentials)
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
 * Generate a random weight in the range [-1, 1].
 *
 * Returns:
 *  - random double between -1 and 1
 */
double rand_weight() { return ((double)rand() / RAND_MAX) * 2.0 - 1.0; }

/**
 * Allocate and initialize a neural network.
 *
 * Parameters:
 *  - input_size  : number of input neurons
 *  - hidden_size : number of neurons in the hidden layer
 *  - output_size : number of output neurons
 *
 * Returns:
 *  - pointer to an allocated NeuralNetwork
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
    {
        nn->hidden_weights[i] = rand_weight();
    }

    for (int i = 0; i < hidden_size * output_size; i++)
    {
        nn->output_weights[i] = rand_weight();
    }

    for (int i = 0; i < hidden_size; i++)
    {
        nn->hidden_bias[i] = rand_weight();
    }

    for (int i = 0; i < output_size; i++)
    {
        nn->output_bias[i] = rand_weight();
    }

    return nn;
}

/**
 * Free all memory allocated for a neural network.
 *
 * Parameters:
 *  - nn : pointer to the NeuralNetwork to destroy
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
 * Save a neural network to a binary file.
 *
 * Parameters:
 *  - nn       : pointer to neural network
 *  - filename : path of the output file
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
 * Load a neural network from a saved file.
 *
 * Parameters:
 *  - filename : path to the saved model
 *
 * Returns:
 *  - pointer to a NeuralNetwork, or NULL on error
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
 *
 * Parameters:
 *  - nn     : pointer to the neural network
 *  - inputs : input array of size input_size
 */
void forward(NeuralNetwork *nn, double *inputs)
{
    for (int i = 0; i < nn->hidden_size; i++)
    {
        double sum = nn->hidden_bias[i];

        for (int j = 0; j < nn->input_size; j++)
        {
            sum += inputs[j] * nn->hidden_weights[j * nn->hidden_size + i];
        }

        nn->hidden[i] = relu(sum);
    }

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

    softmax(logits, nn->output, nn->output_size);

    free(logits);
}

/**
 * Train the neural network using gradient descent and cross entropy loss.
 *
 * Parameters:
 *  - nn       : pointer to neural network
 *  - inputs   : training samples (samples x input_size)
 *  - targets  : expected one-hot outputs (samples x output_size)
 *  - samples  : number of training samples
 *  - lr       : learning rate
 *  - epochs   : number of passes over the dataset
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

            for (int o = 0; o < nn->output_size; o++)
            {
                if (targets[s][o] == 1.0)
                {
                    total_loss += -log(nn->output[o] + 1e-12);
                }
            }

            for (int o = 0; o < nn->output_size; o++)
            {
                output_deltas[o] = nn->output[o] - targets[s][o];
            }

            for (int h = 0; h < nn->hidden_size; h++)
            {
                double sum = 0.0;

                for (int o = 0; o < nn->output_size; o++)
                {
                    sum += output_deltas[o] *
                           nn->output_weights[h * nn->output_size + o];
                }

                hidden_deltas[h] = relu_derivative(nn->hidden[h]) * sum;
            }

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

        if (epoch % 100 == 0)
        {
            printf("Epoch %d - Loss: %.6f\n", epoch, total_loss);
        }
    }
}

/**
 * Predict a letter (A–Z) from a GdkPixbuf image.
 *
 * Parameters:
 *  - nn      : trained neural network
 *  - pixbuf  : grayscaled GdkPixbuf
 *
 * Returns:
 *  - predicted uppercase letter (A–Z)
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
