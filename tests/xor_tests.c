#include <stdio.h>
#include "../include/neural_network.h"

int main() {
    NeuralNetwork *nn = create_network(2, 2, 1);

    double inputs[4][2] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    double targets[4] = {0, 1, 1, 0};

    train(nn, inputs, targets, 0.5, 10000);

    printf("\n--- Tests XOR ---\n");
    int passed = 1;
    for (int i = 0; i < 4; i++) {
        forward(nn, inputs[i]);
        double out = nn->output[0];
        int pred = (out > 0.5) ? 1 : 0;
        printf("Input: %.0f %.0f, Predicted: %d, Expected: %.0f\n",
               inputs[i][0], inputs[i][1], pred, targets[i]);
        if (pred != (int)targets[i]) {
            passed = 0;
        }
    }

    if (passed) {
        printf("All tests passed.\n");
    } else {
        printf("Some tests did not pass.\n");
    }

    free_network(nn);
    return passed ? 0 : 1;
}

