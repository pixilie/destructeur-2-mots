#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/image_rotation.h"

//Return new Pixbuf of rotated image with an angle
GdkPixbuf* rotate_image(GdkPixbuf *src, double angle_degrees)
{
	int width = gdk_pixbuf_get_width(src);
	int height = gdk_pixbuf_get_height(src);
	int rowstride = gdk_pixbuf_get_rowstride(src);
	int n_channels = gdk_pixbuf_get_n_channels(src);
	guchar *pixels = gdk_pixbuf_get_pixels(src);
	
	//Convert degrees to radians
	double angle = angle_degrees * M_PI / 180;
	
	int new_width = (int) (fabs(width * cos(angle)) + fabs(height * sin(angle)));
	int new_height = (int) (fabs(width * sin(angle)) + fabs(height * cos(angle)));

	//Create new Pixbuf for the rotated image
	GdkPixbuf *new = gdk_pixbuf_new(GDK_COLORSPACE_RGB, gdk_pixbuf_get_has_alpha(src), 8, new_width, new_height);	

	int new_rowstride = gdk_pixbuf_get_rowstride(new);	
	int new_n_channels = gdk_pixbuf_get_n_channels(new);
	guchar *new_pixels = gdk_pixbuf_get_pixels(new);
		
	//Fill new Pixbuf with black backrgound
	memset(new_pixels, 0, new_rowstride * new_height);

	double center_x = width / 2.0;
	double center_y = height / 2.0;
	double new_center_x = new_width / 2.0;
	double new_center_y = new_height / 2.0;

	double cos_a = cos(angle);
	double sin_a = sin(angle);

	for(int new_y = 0; new_y < new_height; new_y++)
	{
		for(int new_x = 0; new_x < new_width; new_x++)
		{
			//Find location of pixel in original image
			double x = (new_x - new_center_x) * cos_a + (new_y - new_center_y) * sin_a + center_x;
			double y = -(new_x - new_center_x) * sin_a + (new_y - new_center_y) * cos_a + center_y;
			
			//Copy pixel color if it's inside bounds
			if(x >= 0 && x < width && y >= 0 && y < height)
			{
				int x_i = (int) x;
				int y_i = (int) y;
				
				guchar *pixel = pixels + y_i * rowstride + x_i * n_channels;
				guchar *new_pixel = new_pixels + new_y * new_rowstride + new_x * new_n_channels;

				//Copy pixel (RGB or RGBA)
				for(int c = 0; c < n_channels; c++)
				{
					new_pixel[c] = pixel[c];
				}
			}
		}
	}
	return new;
}
				
				
