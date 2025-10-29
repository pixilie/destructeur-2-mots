#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../include/image_helpers.h"
#include "../../include/image_treatment.h"
#include "../../include/image_rotation.h"
#include "../../include/image_slice.h"
 
#ifndef TESTING

//Executes an image function on a pixbuf
//Optional: Output file at the end
int main(int argc, char **argv) 
{
	if(argc < 3)
	{
		printf("Not enough arguments\n");
		printf("Usage: /image <function_name> <image_path> <function_params> <optional:output>\n");
		return EXIT_FAILURE;
	}

	char *function_name = argv[1];
	
	//Check function name
	if(strcmp(function_name, "convert_to_grayscale") && strcmp(function_name, "binarize_image") 
	&& strcmp(function_name, "rotate_image") && strcmp(function_name, "slice_from") && strcmp(function_name, "slice_in_n"))
	{
		printf("Incorrect function name, got: %s", function_name);
		printf("Usage: ./image <function_name> <image_path> <function_params> <optional:output>\n");
		printf("Available functions:\n");
		printf(" - ./image convert_to_grayscale <image_path> <optional:output>\n");
		printf(" - ./image binarize_image <image_path> <treshold> <optional:output>\n");
		printf(" - ./image rotate_image <image_path> <angle_degrees> <optional:output>\n");
		printf(" - ./image slice_from <image_path> <x> <y> <direction> <optional:outputs>\n");
		printf(" - ./image slice_in_n <image_path> <n_slice> <optional:outputs>\n");
		return EXIT_FAILURE;
	}
	else
	{
		//Load image
		GdkPixbuf *pixbuf = load_image(argv[2]);
		if(!pixbuf)
		{
			printf("[FAIL] Image could not be loaded\n");
			return EXIT_FAILURE;
		}

		//convert_to_grayscale
		if(strcmp(function_name, "convert_to_grayscale") == 0)
		{
			convert_to_grayscale(pixbuf);
			if(argc > 3)
			{
				save_pixbuf_as_png(pixbuf, argv[3]);
			}
			else
			{
				save_pixbuf_as_png(pixbuf, "grayscaled.png");
			}
			g_object_unref(pixbuf);
			return EXIT_SUCCESS;
		}
		
		//binarize_image
		if(strcmp(function_name, "binarize_image") == 0)
		{
			if(argc < 4)
			{
				printf("Not enough arguments for binarize_image\n");
				printf("Usage: ./image binarize_image <image_path> <treshold> <optional:output>\n");
				g_object_unref(pixbuf);
			}
			int treshold = strtol(argv[3], NULL, 10);
			binarize_image(pixbuf, treshold);
			if(argc > 4)
			{
				save_pixbuf_as_png(pixbuf, argv[4]);
			}
			else
			{
				save_pixbuf_as_png(pixbuf, "binarized.png");
			}
			g_object_unref(pixbuf);
			return EXIT_SUCCESS;
		}
		
		//rotate_image
		if(strcmp(function_name, "rotate_image") == 0)
		{
			if(argc < 4)
			{
				printf("Not enough arguments for rotate_image\n");
				printf("Usage: ./image rotate_image <image_path> <angle_degrees> <optional:output>\n");
				g_object_unref(pixbuf);
				return EXIT_FAILURE;
			}
			double angle_degrees = strtod(argv[3], NULL);
			GdkPixbuf *rotated = rotate_image(pixbuf, angle_degrees);
			if(!rotated)
			{
				printf("[FAIL] Rotate function failed\n");
				g_object_unref(pixbuf);
				return EXIT_FAILURE;
			}
			if(argc > 4)
			{
				save_pixbuf_as_png(rotated, argv[4]);
			}
			else
			{
				save_pixbuf_as_png(rotated, "rotated.png");
			}
			g_object_unref(pixbuf);
			g_object_unref(rotated);
			return EXIT_SUCCESS;
		}
		
		//slice_from
		if(strcmp(function_name, "slice_from") == 0)
		{
			if(argc < 6)
			{
				printf("Not enough arguments for slice_from\n");
				printf("Usage: ./image slice_from <image_path> <x> <y> <direction> <optional:output>\n");
				g_object_unref(pixbuf);
				return EXIT_FAILURE;
			}
			int x = (int) strtol(argv[3], NULL, 10);
			int y = (int) strtol(argv[4], NULL, 10);
			int direction = (int) strtol(argv[5], NULL, 10);
			GdkPixbuf **pixbufs = slice_from(pixbuf, x, y, direction);
			GdkPixbuf *pixbuf1 = pixbufs[0];
			GdkPixbuf *pixbuf2 = pixbufs[1];
			if(argc > 6)
			{
				save_pixbuf_as_png(pixbuf1, argv[6]);
				if(argc > 7)
				{
					save_pixbuf_as_png(pixbuf2, argv[7]);
				}
				else
				{
					save_pixbuf_as_png(pixbuf2, "sliced_image2.png");
				}
			}
			else
			{
				save_pixbuf_as_png(pixbuf1, "sliced_image1.png");
				save_pixbuf_as_png(pixbuf2, "sliced_image2.png");
			}
			g_object_unref(pixbuf);
			g_object_unref(pixbufs[0]);
			g_object_unref(pixbufs[1]);
			free(pixbufs);
			return EXIT_SUCCESS;
		}
				
		//slice_in_n	
		if(strcmp(function_name, "slice_in_n") == 0)
		{
			if(argc < 4)
			{
				printf("Not enough arguments for slice_in_n\n");
				printf("Usage: ./image slice_in_n <image_path> <n_slice>\n");
				g_object_unref(pixbuf);
				return EXIT_FAILURE;
			}
			int n_slice = (int) strtol(argv[3], NULL, 10);

			GdkPixbuf **pixbufs = slice_in_n(pixbuf, n_slice);

			char filename[256];
			for(int i = 0; i < n_slice; i++)
			{
				snprintf(filename, sizeof(filename), "sliced_in_n_image%i.png", i);
				save_pixbuf_as_png(pixbufs[i], filename);
				g_object_unref(pixbufs[i]);
			}

			free(pixbufs);
			g_object_unref(pixbuf);
			return EXIT_SUCCESS;
		}
		
		g_object_unref(pixbuf);
		return EXIT_SUCCESS;
	}	
}
#endif
