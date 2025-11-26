#include <stdio.h>

#include "../include/dataset.h"
#include "../include/neural_network.h"
#include "../include/test_helpers.h"

#define DATASET_PATH "./tests/dataset/"
#define MODEL_PATH "./tests/model.nn"

int argmax(double *arr, int size)
{
    int best = 0;
    for (int i = 1; i < size; i++)
    {
        if (arr[i] > arr[best])
            best = i;
    }
    return best;
}

int main()
{
    print_test_category("Letter Recognition Neural Network");

    print_test_subcategory("Loading Dataset");
    Dataset data = load_dataset(DATASET_PATH);
    if (data.samples == 0)
    {
        print_fail();
        printf("ERROR: No dataset found in %s\n", DATASET_PATH);
        return 1;
    }
    print_success();
    printf("Loaded %d samples (input size: %d)\n", data.samples,
           data.input_size);

    print_test_subcategory("Creating Network");
    NeuralNetwork *nn = create_network(data.input_size, 128, 26);
    if (!nn)
    {
        print_fail();
        printf("Failed to allocate neural network.\n");
        return 1;
    }
    print_success();
    printf("Network created: %d-%d-%d\n", nn->input_size, nn->hidden_size,
           nn->output_size);

    print_test_subcategory("Training Neural Network");
    train(nn, data.inputs, data.targets, data.samples, 0.01, 500);
    print_success();
    printf("Training completed successfully.\n");

    print_test_subcategory("Saving Model");
    save_network(nn, MODEL_PATH);
    print_success();
    printf("Model saved to %s\n", MODEL_PATH);

    print_test_subcategory("Evaluating on Training Dataset");
    int correct = 0;
    for (int i = 0; i < data.samples; i++)
    {
        forward(nn, data.inputs[i]);
        int predicted = argmax(nn->output, nn->output_size);
        int expected = data.labels[i];

        if (predicted == expected)
        {
            print_success();
            printf("Sample %d: predicted %c — expected %c\n", i,
                   'A' + predicted, 'A' + expected);
            correct++;
        }
        else
        {
            print_fail();
            printf("Sample %d: predicted %c — expected %c\n", i,
                   'A' + predicted, 'A' + expected);
        }
    }

    double accuracy = (double)correct / data.samples * 100.0;
    printf("\nAccuracy = %.2f%%\n", accuracy);

    if (accuracy > 90.0)
    {
        print_all_tests_passed("Letter Recognition Neural Network");
    }
    else
    {
        print_some_tests_failed("Letter Recognition Neural Network");
    }

    free_network(nn);
    free_dataset(&data);
    return 0;
}
