#include "../include/neural_network.h"
#include "../include/test_helpers.h"

#include <stdio.h>

int test_xor()
{
    print_test_subcategory("XOR Tests");

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

        if (pred != (int)targets[i])
        {
            passed = 0;
            print_fail();
        }
        else
        {
            print_success();
        }

        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
    }

    if (passed)
    {
        print_success();
        printf("All tests passed for XOR\n");
    }
    else
    {
        print_fail();
        printf("Some tests did not pass for XOR\n");
    }

    free_network(nn_xor);

    return passed;
}

int test_and()
{
    print_test_subcategory("AND (A.B) Tests");

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

        if (pred != (int)targets[i])
        {
            passed = 0;
            print_fail();
        }
        else
        {
            print_success();
        }
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
    }

    if (passed)
    {
        print_success();
        printf("All tests passed for AND\n");
    }
    else
    {
        printf("Some tests did not pass for AND\n");
    }
    free_network(nn_and);

    return passed;
}

int test_or()
{
    print_test_subcategory("OR (A+B) Tests");

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

        if (pred != (int)targets[i])
        {
            passed = 0;
            print_fail();
        }
        else
        {
            print_success();
        }
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
    }

    if (passed)
    {
        print_success();
        printf("All tests passed for OR\n");
    }
    else
    {
        print_fail();
        printf("Some tests did not pass for OR\n");
    }
    free_network(nn_or);

    return passed;
}

int test_xnor()
{
    print_test_subcategory("XNOR (!A.!B + A.B) Tests");

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

        if (pred != (int)targets[i])
        {
            passed = 0;
            print_fail();
        }
        else
        {
            print_success();
        }
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
    }

    if (passed)
    {
        print_success();
        printf("All tests passed for XNOR\n");
    }
    else
    {
        print_fail();
        printf("Some tests did not pass for XNOR\n");
    }
    free_network(nn_xnor);

    return passed;
}

int main()
{
    print_test_category("Neural Network Tests");

    int passed = test_and() && test_or() && test_xnor() && test_xor();

    if (passed)
    {
        print_all_tests_passed("Neural Network Tests");
    }
    else
    {
        print_some_tests_failed("Neural Network Tests");
    }

    return passed ? 0 : 1;
}
