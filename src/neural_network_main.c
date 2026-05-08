#include <stdio.h>
#include <stdlib.h>

#include "../include/grid_cutting.h"
#include "../include/neural_network.h"
#include "../include/image/image_helpers.h"

#define DEFAULT_NEURAL_PATH "assets/ocr_model"

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"
#define COLOR_LIGHT_BLUE "\033[36m"

const double YELLOW_PROBABILITY = 5; // Minimum probability in % to display as yellow, else red

/*
 * predict_letter_main:
 * Predict a single letter (A-Z) from a GdkPixbuf image. Returns uppercase char.
 * Also prints the probabilities for every letter.
 */
char predict_letter_main(NeuralNetwork *nn, GdkPixbuf *pixbuf)
{
  GdkPixbuf *scaled_pixbuf = scale_pixbuf_to_28x28(pixbuf);

  double input[28 * 28];
  pixbuf_to_input_vector(scaled_pixbuf, input);

  forward(nn, input);
  g_object_unref(scaled_pixbuf);

  int best = 0;
  double best_val = nn->output[0];

  // Predict letter (find best probability)
  for (int i = 0; i < nn->output_size; i++)
  {
    if (nn->output[i] > best_val)
    {
      best_val = nn->output[i];
      best = i;
    }
  }

  // Print letters A -> M
  for (int i = 0; i < nn->output_size / 2; i++)
  {
    char letter = (char)('A' + i);
    printf(COLOR_LIGHT_BLUE "  %c    " COLOR_RESET, letter);
  }
  printf("\n");

  // Print probabilities A -> M
  for (int i = 0; i < nn->output_size / 2; i++)
  {
    double probability = nn->output[i] * 100;
    if (i == best)
    {
      printf(COLOR_GREEN "%.1f%%   " COLOR_RESET, probability);
    }
    else if (probability >= YELLOW_PROBABILITY)
    {
      printf(COLOR_YELLOW "%.1f%%   " COLOR_RESET, probability);
    }
    else
    {
      printf(COLOR_RED "%.1f%%   " COLOR_RESET, probability);
    }
  }
  printf("\n");

  // Print letters N -> Z
  for (int i = nn->output_size / 2; i < nn->output_size; i++)
  {
    char letter = (char)('A' + i);
    printf(COLOR_LIGHT_BLUE "  %c    " COLOR_RESET, letter);
  }
  printf("\n");

  // Print probabilities N -> Z
  for (int i = nn->output_size / 2; i < nn->output_size; i++)
  {
    double probability = nn->output[i] * 100;
    if (i == best)
    {
      printf(COLOR_GREEN "%.1f%%   " COLOR_RESET, probability);
    }
    else if (probability >= YELLOW_PROBABILITY)
    {
      printf(COLOR_YELLOW "%.1f%%   " COLOR_RESET, probability);
    }
    else
    {
      printf(COLOR_RED "%.1f%%   " COLOR_RESET, probability);
    }
  }
  printf("\n");

  char predicted_letter = (char)('A' + best);
  printf(COLOR_GREEN "✔ [SUCCESS] " COLOR_RESET "Predicted letter "
    COLOR_YELLOW "%c " COLOR_RESET "with a probability of "
    COLOR_YELLOW "%.1f%%\n" COLOR_RESET, predicted_letter, best_val * 100);
  return predicted_letter;
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    printf("Usage : ./neural <cropped_letter_image>\n");
    return EXIT_FAILURE;
  }

  char neural_path[1024];
  char *exe_dir = get_executable_dir();
  snprintf(neural_path, sizeof(neural_path), "%s/../%s", exe_dir,
           DEFAULT_NEURAL_PATH);

  NeuralNetwork *nn = load_network(neural_path);
  if (!nn)
  {
    printf(COLOR_RED "✘ [ERROR] " COLOR_RESET "Failed to load neural network at " COLOR_YELLOW "%s\n" COLOR_RESET, neural_path);
    return EXIT_FAILURE;
  }
  printf(COLOR_GREEN "✔ [SUCCESS] " COLOR_RESET "Loaded neural network at " COLOR_YELLOW "%s\n" COLOR_RESET, neural_path);

  char image_path[1024];
  snprintf(image_path, sizeof(image_path), "%s/../%s", exe_dir,
           argv[1]);

  GdkPixbuf* pixbuf = load_image(image_path);
  if (!pixbuf)
  {
    printf(COLOR_RED "✘ [ERROR] " COLOR_RESET "Couldn't load image at " COLOR_YELLOW "%s\n" COLOR_RESET, image_path);
    free_network(nn);
    return EXIT_FAILURE;
  }
  printf(COLOR_GREEN "✔ [SUCCESS] " COLOR_RESET "Loaded image at " COLOR_YELLOW "%s\n" COLOR_RESET, image_path);

  predict_letter_main(nn, pixbuf);

  g_object_unref(pixbuf);
  free_network(nn);

  return EXIT_SUCCESS;
}