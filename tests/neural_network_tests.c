#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/dataset.h"
#include "../include/image/image.h"
#include "../include/neural_network.h"
#include "../include/test_helpers.h"

#define DATASET_PATH "./tests/dataset/"
#define MODEL_PATH "./tests/model"

int get_test_image_path(const char *base_path, char letter, char *out_path)
{
    char dir_path[256];
    sprintf(dir_path, "%s/%c", base_path, letter);

    DIR *d = opendir(dir_path);
    if (!d)
        return 0;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL)
    {
        if (strstr(entry->d_name, ".png"))
        {
            sprintf(out_path, "%s/%s", dir_path, entry->d_name);
            closedir(d);
            return 1;
        }
    }

    closedir(d);
    return 0;
}

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
    train(nn, data.inputs, data.targets, data.samples, 0.01, 1000);
    print_success();
    printf("Training completed successfully.\n");

    print_test_subcategory("Saving Model");
    save_network(nn, MODEL_PATH);
    print_success();
    printf("Model saved to %s\n", MODEL_PATH);

    print_test_subcategory("Evaluating on Training Dataset (Vectors)");
    int correct = 0;
    for (int i = 0; i < data.samples; i++)
    {
        forward(nn, data.inputs[i]);
        int predicted = argmax(nn->output, nn->output_size);
        int expected = data.labels[i];

        if (predicted == expected)
        {
            correct++;
        }
    }

    double accuracy = (double)correct / data.samples * 100.0;

    if (accuracy > 90.0)
    {
        print_success();
    }
    else
    {
        print_fail();
    }

    printf("Global Accuracy = %.2f%%\n", accuracy);

    print_test_subcategory("Testing function predict_letter() with Images");

    char letters_to_test[] = {'A', 'B', 'C', 'I', 'J', 'M', 'Z'};

    for (unsigned long i = 0; i < sizeof(letters_to_test); i++)
    {
        char target = letters_to_test[i];
        char image_path[512];

        if (get_test_image_path(DATASET_PATH, target, image_path))
        {
            GdkPixbuf *img = load_image(image_path);

            if (img)
            {
                char prediction = predict_letter(nn, img);

                if (prediction == target)
                {
                    print_success();
                    printf("Image '%c' -> Predicted '%c'\n", target,
                           prediction);
                }
                else
                {
                    print_fail();
                    printf("Image '%c' -> Predicted '%c' (File: %s)\n", target,
                           prediction, image_path);
                }

                g_object_unref(img);
            }
            else
            {
                printf("Warning: Could not load image %s\n", image_path);
            }
        }
    }

    free_network(nn);
    free_dataset(&data);
    return 0;
}
