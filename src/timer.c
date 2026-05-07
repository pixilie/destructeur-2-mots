#include "../include/timer.h"
#include <math.h>
#include <stdio.h>

const char* timer_icon = "⌛";
#define COLOR_LIGHT_BLUE "\033[36m"
#define COLOR_RESET "\033[0m"

clock_t start_timer() { return clock(); }

float get_timer(clock_t start_timer)
{
  clock_t processor_time = clock() - start_timer;
  float time = (float) processor_time / CLOCKS_PER_SEC;
  return roundf(time * 1000) / 1000;
}

void print_time(float time)
{
  printf(COLOR_LIGHT_BLUE " [%s %.2f s]\n" COLOR_RESET, timer_icon, time);
}