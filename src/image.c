// Include gtk
#include <gtk/gtk.h>
#include <libgen.h>  // pour dirname
#include <limits.h>  // PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // readlink

static void get_image_path()
{
	//Copy image path code here
}

static void on_activate (GtkApplication *app) 
{
	//UI Variables
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *button;
	GtkWidget *image;
	GtkWidget *scrolled;

	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) 
	{
		g_print("Pwd: %s\n", cwd);
	}

	// Create a new window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Image");
	gtk_window_set_default_size(GTK_WINDOW(window), 2000, 1000);

	// Create a vertical box
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

	// Get absolute path of binary
	char exe_path[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
	if (len == -1) 
	{
		g_print("ERROR: Failed to read binary path\n");
		return;
	}
	exe_path[len] = '\0';

	// Get directory of binary
	char *dir = dirname(exe_path);

	// Build absolute path of binary
	char image_path[PATH_MAX];
	snprintf(image_path, sizeof(image_path), "%s/%s", dir, "../assets/level_1_image_1.png");
	g_print("Image path : %s\n", image_path);


	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
	if (!pixbuf)
	{
		g_print("ERROR: Failed to load image\n");
		return;
	}

	// Load image from file
	GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, 1000, 700, GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf(scaled);

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

