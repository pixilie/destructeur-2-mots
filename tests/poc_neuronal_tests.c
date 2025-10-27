#include "../include/neural_network.h"
#include <stdio.h>

int test_xor()
{
    printf("\n--- Tests XOR ---\n");

    NeuralNetwork *nn_xor = create_network(2, 2, 1);

    double inputs[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double targets[4] = {0, 1, 1, 0};

    train(nn_xor, inputs, targets, 0.5, 10000);

    int passed = 1;
    for (int i = 0; i < 4; i++)
    {
        forward(nn_xor, inputs[i]);
        double out = nn_xor->output[0];
        int pred = (out > 0.5) ? 1 : 0;
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
        if (pred != (int)targets[i])
        {
            passed = 0;
        }
    }

    if (passed)
    {
        printf("All tests passed.\n");
    }
    else
    {
        printf("Some tests did not pass.\n");
    }

    free_network(nn_xor);

    return passed;
}

int test_and()
{
    printf("\n--- Tests AND (A.B) ---\n");

    NeuralNetwork *nn_and = create_network(2, 2, 1);

    double inputs[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double targets[4] = {0, 0, 0, 1};

    train(nn_and, inputs, targets, 0.5, 10000);

    int passed = 1;
    for (int i = 0; i < 4; i++)
    {
        forward(nn_and, inputs[i]);
        double out = nn_and->output[0];
        int pred = (out > 0.5) ? 1 : 0;
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
        if (pred != (int)targets[i])
        {
            passed = 0;
        }
    }

    if (passed)
    {
        printf("All tests passed.\n");
    }
    else
    {
        printf("Some tests did not pass.\n");
    }
    free_network(nn_and);

    return passed;
}

int test_or()
{
    printf("\n--- Tests OR (A+B) ---\n");

    NeuralNetwork *nn_or = create_network(2, 2, 1);

    double inputs[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double targets[4] = {0, 1, 1, 1};

    train(nn_or, inputs, targets, 0.5, 10000);

    int passed = 1;
    for (int i = 0; i < 4; i++)
    {
        forward(nn_or, inputs[i]);
        double out = nn_or->output[0];
        int pred = (out > 0.5) ? 1 : 0;
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
        if (pred != (int)targets[i])
        {
            passed = 0;
        }
    }

    if (passed)
    {
        printf("All tests passed.\n");
    }
    else
    {
        printf("Some tests did not pass.\n");
    }
    free_network(nn_or);

    return passed;
}

int test_xnor()
{
    printf("\n--- Tests XNOR (!A.!B + A.B) ---\n");

    NeuralNetwork *nn_xnor = create_network(2, 2, 1);

    double inputs[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double targets[4] = {1, 0, 0, 1};

    train(nn_xnor, inputs, targets, 0.5, 10000);

    int passed = 1;
    for (int i = 0; i < 4; i++)
    {
        forward(nn_xnor, inputs[i]);
        double out = nn_xnor->output[0];
        int pred = (out > 0.5) ? 1 : 0;
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
        if (pred != (int)targets[i])
        {
            passed = 0;
        }
    }

    if (passed)
    {
        printf("All tests passed.\n");
    }
    else
    {
        printf("Some tests did not pass.\n");
    }
    free_network(nn_xnor);

    return passed;
}

int main()
{
    int passed = test_and() && test_or() && test_xnor() && test_xor();
    return passed ? 0 : 1;
}
