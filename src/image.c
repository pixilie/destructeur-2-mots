// Include gtk
#include <gtk/gtk.h>
#include <libgen.h>  // dirname
#include <limits.h>  // PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // readlink
#include <gdk-pixbuf/gdk-pixbuf.h> //access to image pixels

char* get_image_path(const char* filename) //Return absolute path of image in assets
{
	static char image_path[PATH_MAX];

	//Read absolute path of binary
	char exe_path[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
	if (len == -1) 
	{
		g_printerr("ERROR: Failed to read binary path\n");
		return NULL;
	}
	exe_path[len] = '\0';

	// Get directory of binary
	char *dir = dirname(exe_path);
	
	snprintf(image_path, sizeof(image_path), "%s/../assets/%s", dir, filename); //../assets/<filename>

	g_print("Image path : %s\n", image_path);
	return image_path;
}

void convert_to_greyscale(GdkPixbuf *pixbuf) //Convert colored image (RGB or RGBA) in grayscale
{
	//Formula for grayscale: Gray = 0.299 × R + 0.587 × G + 0.114 × B
	
	//Get image dimensions
	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	
	//Get channels per pixel: (R, G, B) = 3, (R, G, B, A) = 4
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	
	//Get array of pixeks
	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	
	//Get rowstride = bytes between 2 lines (can have unused bytes at the end of each row)
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	for(int y = 0; y < height; y++)
	{
		guchar *rows = pixels + y * rowstride;
		for(int x = 0; x < width; x++)
		{
			guchar *pixel = rows + x * n_channels;
			guchar r = pixel[0]; //R
			guchar g = pixel[1]; //G
			guchar b = pixel[2]; //B
			
			//Calculate gray formula and make every R, G and B the same grayness
			guchar gray = (guchar) (0.299 * r + 0.587 * g + 0.114 * b);
			for(int i = 0; i < 3; i++)
			{
				pixel[i] = gray;
			}
		}
	}
}

void binarize_image(GdkPixbuf *pixbuf, int treshold)
{
	//Transform gray image into a black and white image
	//For each pixel in image: 
	// - if gray < treshold -> black pixel
	// - if gray >= treshold -> white pixel
	//Treshold is 128 by default

	//Get image dimensions
	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	
	//Get channels per pixel: (R, G, B) = 3, (R, G, B, A) = 4
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	
	//Get array of pixeks
	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
	
	//Get rowstride = bytes between 2 lines (can have unused bytes at the end of each row)
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	for(int y = 0; y < height; y++)
	{
		guchar *rows = pixels + y * rowstride;
		for(int x = 0; x < width; x++)
		{
			guchar *pixel = rows + x * n_channels;
			guchar gray = pixel[0];
			if(gray < treshold)
			{
				gray = 0; //Black pixel
			}
			else
			{
				gray = 255; //White pixel
			}
			for(int i = 0; i < 3; i++)
			{
				pixel[i] = gray;
			}
		}
	}
}

static void on_activate (GtkApplication *app) 
{
	//UI Variables
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *button;
	GtkWidget *image;
	GtkWidget *scrolled;

	// Create a new window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Image");
	gtk_window_set_default_size(GTK_WINDOW(window), 2000, 1000);

	// Create a vertical box
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

	const char* image_path = get_image_path("level_1_image_1.png");
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
	if (!pixbuf)
	{
		g_printerr("ERROR: Failed to load image at %s\n", image_path);
		return;
	}

	//Convert image in levels of gray
	convert_to_greyscale(pixbuf);
	
	//Convert image in black and white
	binarize_image(pixbuf, 180);

	// Load image from file
	GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, 1000, 700, GDK_INTERP_BILINEAR);

	image = gtk_image_new_from_pixbuf(scaled);
	
	//For testing only: Save black and white image in new file output.png"
	gdk_pixbuf_save(scaled, "output.png", "png", NULL, NULL);


	if (!gtk_image_get_pixbuf(GTK_IMAGE(image))) 
	{
		g_print("ERROR: Image not loaded. Check image path\n");
		return;
	}

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			       GTK_POLICY_AUTOMATIC,
			       GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolled), image);

	// Create a new button
	button = gtk_button_new_with_label("Hello, World!");
	// When the button is clicked, close the window passed as an argument
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_close), window);

	// Add scrolled + button in box
	gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);

	// Add box to window
	gtk_container_add(GTK_CONTAINER(window), box);

	//Show all widgets
	gtk_widget_show_all(window);

	//Free pixbufs
	g_object_unref(pixbuf);
	g_object_unref(scaled);
}

void sliceFrom(int x, int y, int direction)
{
    //Slice an image int two parts at the coords X,Y
    //and can slice horizontally (direction == 1) or vertically
    //Save the 2 parts sliced : left / right or high / low

    //Obtain path of img and pixbuf of the img
    const char* image_path = get_image_path("level_1_image_1.png");
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
    if (!pixbuf)
    {
            g_printerr("ERROR: Failed to load image at %s\n", image_path);
            return;
    }

    //Get image dimensions
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    if(direction == 1) //we want to cut horizontally --> x useless
    {
        int new_height = height - y;
        GdkPixbuf * high = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, width, y);
        GdkPixbuf * low = gdk_pixbuf_new_subpixbuf(pixbuf, 0, y, width, new_height);

        //save the new img
        if(!gdk_pixbuf_save(high, "High_img", "png", NULL, NULL))
        {
            g_print("ERROR : error occured in saving High_img as png\n");
            return;
        }
	}
    if(!gdk_pixbuf_save(low, "Low_img", "png", NULL, NULL))
	{
        {
            g_print("ERROR : error occured in saving Low_img as png\n");
            return;
        }
    }
    else //we want to cut vertically --> y useless
    {
        int new_width = width - x;
        GdkPixbuf * left = gdk_pixbuf_new_subpixbuf(pixbuf, 0, 0, x, height);
        GdkPixbuf * right = gdk_pixbuf_new_subpixbuf(pixbuf, x, 0, new_width, height);


        //save the new img
        if(!gdk_pixbuf_save(left, "Left_img", "png", NULL, NULL))
        {
            g_print("ERROR : error occured in saving left img as png\n");
            return;
        }
        if(!gdk_pixbuf_save(right, "Right_img", "png", NULL, NULL))
        {
            g_print("ERROR : error occured in saving right img as png\n");
            return;
        }
    }
}


void slice_in_n(GdkPixbuf * pixbuf, int n_slice)
{
    //Slice a SQUARE image in n_slice square, which
    //are saved as img.png


    //Get image dimensions, width = height because square
    int width = gdk_pixbuf_get_width(pixbuf);

    //Get the size of each char we will save as file
    int sizeEach = width / n_slice;

    //Loop to slice n_slice times
    for(int i = 0; i < n_slice; i++)
    {
        GdkPixbuf * col = gdk_pixbuf_new_subpixbuf(pixbuf, sizeEach * i, 0,
                        sizeEach * (i + 1), width); //we get columns

        for(int j = 0; j < n_slice; j++)
        {
            int x_coord = sizeEach * i;
            GdkPixbuf * square = gdk_pixbuf_new_subpixbuf(col, x_coord,
                            sizeEach * i, x_coord, sizeEach * (i + 1));

            if(!gdk_pixbuf_save(square, "char%i", "png", NULL))
            {
                g_print("ERROR : error occured in saving char as png\n");
                return;
            }
        }
    }
}

int main (int argc, char *argv[]) 
{
	GtkApplication *app;
	int status;

	// Create a new application
	app = gtk_application_new("com.example.GtkApplication",
					     G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}

