#include "../include/neural_network.h"
#include "image/image_helpers.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * relu:
 * Rectified Linear Unit activation. Returns x if x > 0, else 0.
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

/*
 * relu_derivative:
 * Derivative of ReLU. Returns 1 if x > 0, else 0.
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

/*
 * softmax:
 * Compute softmax of input logits into output array (numerically stable).
 * Parameters:
 *  - input: logits array
 *  - output: array to receive probabilities
 *  - size: number of elements
 * Returns:
 *  - sum of exponentials (normalization factor)
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

/*
 * init_weights_he:
 * Initialize weights with He (Kaiming) uniform initialization.
 * Parameters:
 *  - weights: array to fill
 *  - size: number of weights
 *  - fan_in: number of input units
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

/*
 * create_network:
 * Allocate and initialize a NeuralNetwork with given sizes.
 * Weights are initialized and biases set to zero.
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

/*
 * free_network:
 * Free memory of a NeuralNetwork instance.
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

/*
 * save_network:
 * Save network sizes, weights and biases to a binary file.
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

/*
 * load_network:
 * Load a network saved with save_network. Returns allocated NeuralNetwork or
 * NULL.
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
    if (fread(&input, sizeof(int), 1, f) != 1 ||
        fread(&hidden, sizeof(int), 1, f) != 1 ||
        fread(&output, sizeof(int), 1, f) != 1)
    {

        fprintf(stderr, "Error: Failed to read network dimension ");
        fclose(f);
        return NULL;
    }

    NeuralNetwork *nn = create_network(input, hidden, output);

    size_t expected_hidden_weights = input * hidden;
    if (fread(nn->hidden_weights, sizeof(double), expected_hidden_weights, f) !=
        expected_hidden_weights)
    {
        fprintf(stderr, "An error occured while reading hidden_weights\n");
        fclose(f);
        return NULL;
    }

    size_t expected_output_weights = hidden * output;
    if (fread(nn->output_weights, sizeof(double), expected_output_weights, f) !=
        expected_output_weights)
    {
        fprintf(stderr, "An error occured while reading output_weights\n");
        fclose(f);
        return NULL;
    }

    if (fread(nn->hidden_bias, sizeof(double), hidden, f) != (size_t)hidden)
    {
        fprintf(stderr, "An error occured while reading hidden_weights\n");
        fclose(f);
        return NULL;
    }

    if (fread(nn->output_bias, sizeof(double), output, f) != (size_t)output)
    {
        fprintf(stderr, "An error occurend while reading output_bias\n");
        fclose(f);
        return NULL;
    }

    fclose(f);
    return nn;
}

/*
 * forward:
 * Run a forward pass: input -> hidden (linear + ReLU) -> output (linear +
 * softmax). Stores results in nn->hidden and nn->output.
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

/*
 * train:
 * Train network using SGD over provided samples (no batching).
 * Parameters:
 *  - nn: network
 *  - inputs: array of input vectors
 *  - targets: array of one-hot target vectors
 *  - samples: number of samples
 *  - lr: learning rate
 *  - epochs: number of epochs
 */
void train(NeuralNetwork *nn, double **inputs, double **targets, int samples,
           double lr, int epochs, ProgressCallback cb, void *user_data)
{
    for (int epoch = 0; epoch < epochs; epoch++)
    {
        if (cb != NULL)
        {
            cb(epoch + 1, epochs, user_data);
        }

        for (int s = 0; s < samples; s++)
        {
            forward(nn, inputs[s]);

            double *output_deltas = malloc(sizeof(double) * nn->output_size);
            double *hidden_deltas = malloc(sizeof(double) * nn->hidden_size);

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
    }
}

/*
 * predict_letter:
 * Predict a single letter (A-Z) from a GdkPixbuf image. Returns uppercase char.
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