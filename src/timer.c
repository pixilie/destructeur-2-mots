#include "../include/timer.h"
#include <math.h>

clock_t start_timer() { return clock(); }

float get_timer(clock_t start_timer)
{
  clock_t processor_time = clock() - start_timer;
  float time = (float) processor_time / CLOCKS_PER_SEC;
  return roundf(time * 1000) / 1000;
}
