#include <stdlib.h>

#ifndef TESTING

//Executes an image function on a pixbuf
//Optional: Output file at the end, if not: "output.png" by default
int main(int argc, char **argv) 
{
	if(argc < 2)
	{
		printf("Not enough arguments\n");
		printf("Usage: /image <function-name> <image-path> <function-params> <optional:output>\n");
		return EXIT_FAILURE;
	}
	else
	{
		char *function_name = argv[1];
		if(function_name != "convert_to_grayscale" && function_name != "binarize_image" 
		&& function_name != "rotate_image" && function_name != "slice_from" && function_name != "slice_in_n")
		{
			printf("Incorrect function name, got: %s", function_name);
			printf("Usage: ./image <function-name> <image-path> <function-params> <optional:output>\n");
			printf("Available functions:\n");
			printf("- convert_to_grayscale(GdkPixbuf *pixbuf)\n");
			printf(" - binarize_image(GdkPixbuf *pixbuf)\n");
			printf(" - rotate_image(GdkPixbuf *pixbuf, double angle_degrees)\n");
			printf("slice_from(GdkPixbuf *pixbuf)\n");
			printf("slice_in_n(GdkPixbuf *pixbuf)\n");
			return EXIT_FAILURE;
		}
		else
		{
			//convert_to_grayscale
			if(strcmp(function_name, "convert_to_grayscale") == 0)
			{
				return EXIT_SUCCESS;
			}
			
			//binarize_image
			if(strcmp(function_name, "binarize_image") == 0)
			{
				return EXIT_SUCCESS;
			}
			
			//rotate_image
			if(strcmp(function_name, "rotate_image") == 0)
			{
				if(argc < 3)
				{
					printf("Not enough arguments\n");
					printf("Usage: ./image rotate_image <angle_degrees> <optional:output>\n");
					return EXIT_FAILURE;
				}
				return EXIT_SUCCESS;
			}
		}
	}	
}
#endif
