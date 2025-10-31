#include "../include/line_detection.h"

int test_pipeline(char *filename, char *output_gw_file, char *output_letter_file)
{
	pipeline(filename, output_gw_file, output_letter_file);
}

int main()
{
	int test = test_pipeline("../assets/level_1_image_1","result/grid_and_word","result/letter")   
}
