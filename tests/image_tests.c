#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "../include/image.h"

int test_load_image()
{
	printf("\n--- Load Image Tests ---\n");

	GError *error = NULL;
	const char *path = get_image_path("level_1_image_1.png");
	if(!path)
	{
		printf("[FAIL] Failed to load image\n");
		return 0;
	}
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, &error);

	if(pixbuf == NULL)
	{
		printf("[FAIL] Failed to load image : %s\n", error->message);
		if(error)
		{
			g_error_free(error);
		}
		return 0;
	}

	printf("[SUCCESS] Successfully loaded image (%dx%d)\n",
	   gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));

	g_object_unref(pixbuf);
	return 1;
}

int test_grayscale()
{
	printf("\n--- Grayscale Tests ---\n");

	const char *path = get_image_path("level_1_image_1.png");
	if(!path)
	{
		printf("[FAIL] Failed to load image\n");
		return 0;
	}
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if(!pixbuf)
	{
		printf("[FAIL] Failed to load image\n");
		return 0;
	}

	convert_to_grayscale(pixbuf);
	if(!pixbuf)
	{
		printf("[FAIL] Grayscale failed\n");
		g_object_unref(pixbuf);
		return 0;
	}

	//Check that R = G = B for each pixel (levels of gray)
	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

	int ok = 1;
	for(int y = 0; y < height && ok; y++)
	{
		guchar *row = pixels + y * rowstride;
		for(int x = 0; x < width; x++)
		{
			guchar *p = row + x * n_channels;
			if(!(p[0] == p[1] && p[1] == p[2]))
			{
				ok = 0;
				break;
			}
		}
	}

	if(ok)
	{
		printf("[SUCCESS] Grayscale success\n");
	}
	else
	{
		printf("[FAIL] Grayscale failed\n");
	}
	g_object_unref(pixbuf);
	return ok;
}

int test_black_and_white()
{
	printf("\n--- Binarize Tests ---\n");

	const char *path = get_image_path("level_1_image_1.png");
	if(!path)
	{
		printf("[FAIL] Failed to load image\n");
		return 0;
	}
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
	if(!pixbuf)
	{
		printf("[FAIL] Failed to load image\n");
		return 0;
	}

	convert_to_grayscale(pixbuf);
	if(!pixbuf)
	{
		printf("[FAIL] Grayscale failed\n");
		g_object_unref(pixbuf);
		return 0;
	}

	binarize_image(pixbuf, 128);
	if(!pixbuf)
	{
		printf("[FAIL] Binarize failed");
		g_object_unref(pixbuf);
		return 0;
	}

	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

	int ok = 1;
	for(int y = 0; y < height && ok; y++)
	{
		guchar *row = pixels + y * rowstride;
		for(int x = 0; x < width; x++)
		{
			guchar *p = row + x * n_channels;
			if(!((p[0] == 0 && p[1] == 0 && p[2] == 0) ||
			(p[0] == 255 && p[1] == 255 && p[2] == 255)))
			{
				ok = 0;
				break;
			}
		}
	}

	if (ok)
	{
		printf("[SUCCESS] Successfully converted image to black and white\n");
	}
	else
	{
		printf("[FAIL] Binarize failed\n");
	}
	g_object_unref(pixbuf);
	return ok;
}

int main()
{
	if (chdir("..") != 0) 
	{
        	perror("chdir failed");
        	return 1;
    	}

	int passed = 1;

	passed &= test_load_image();
	passed &= test_grayscale();
	passed &= test_black_and_white();

	printf("\nImage Tests : %s\n", passed ? "All tests passed" : "Some tests did not pass");

	return passed ? 0 : 1;
}
