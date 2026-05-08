#include <stdio.h>
#include <stdlib.h>

#include "../include/grid_cutting.h"
#include "../include/neural_network.h"
#include "../include/image/image_helpers.h"

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"
#define COLOR_LIGHT_BLUE "\033[36m"

#define DEFAULT_NEURAL_PATH "assets/ocr_model"

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    printf(COLOR_YELLOW "[INFO] " COLOR_RESET "Usage : " COLOR_LIGHT_BLUE "./pipeline <image>\n" COLOR_RESET);
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

  PipelineResult *pipeline_result = pipeline(image_path, nn);
  if (!pipeline_result)
  {
    free_network(nn);
    return EXIT_FAILURE;
  }

  free_pipeline(pipeline_result);
  free_network(nn);
  return EXIT_SUCCESS;
}