#include "../include/image_helpers.h"
#include "../include/image_rotation.h"
#include "../include/image_treatment.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void on_activate(GtkApplication *app) __attribute__((unused));

/**
 * on_activate:
 * Signal handler for the GtkApplication "activate" event.
 *
 * Parameters:
 *  - app: pointer to the GtkApplication that emitted the activate signal.
 *
 * Behavior / Side effects:
 *  - Creates the main application window and its child widgets (a scrolled
 *    container with an image and a button).
 *  - Loads an image path via get_image_path(), reads the image into a
 * GdkPixbuf, converts it to grayscale, binarizes it and scales it for display.
 *  - Saves intermediate results to "output.png" (binarized scaled image) and
 *    "rotated.png" (rotated copy) for debugging/testing.
 *  - Connects the button to close the window when clicked.
 *  - Frees created GdkPixbuf objects and shows all widgets.
 *
 * Notes / Constraints:
 *  - Uses hard-coded scale dimensions (1000x700) and binarization threshold
 * (180).
 *  - Prints errors with g_printerr/g_print and returns early on failures.
 *  - The function owns and releases local GdkPixbuf references via
 * g_object_unref().
 */
static void on_activate(GtkApplication *app)
{
    // UI Variables
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

    const char *image_path = get_image_path("level_1_image_1.png");
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
    if (!pixbuf)
    {
        g_printerr("ERROR: Failed to load image at %s\n", image_path);
        return;
    }

    // Convert image in levels of gray
    convert_to_grayscale(pixbuf);

    // Convert image in black and white
    binarize_image(pixbuf, 180);

    // Load image from file
    GdkPixbuf *scaled =
        gdk_pixbuf_scale_simple(pixbuf, 1000, 700, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(scaled);

    // For testing only: Save black and white image in new file output.png"
    gdk_pixbuf_save(scaled, "output.png", "png", NULL, NULL);

    if (!gtk_image_get_pixbuf(GTK_IMAGE(image)))
    {
        g_print("ERROR: Image not loaded. Check image path\n");
        return;
    }

    // Rotate image by 45 degrees and save rotated image in rotated.png
    GdkPixbuf *rotated = rotate_image(scaled, 45.0);
    gdk_pixbuf_save(rotated, "rotated.png", "png", NULL, NULL);
    g_object_unref(rotated);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), image);

    // Create a new button
    button = gtk_button_new_with_label("Hello, World!");
    // When the button is clicked, close the window passed as an argument
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_close),
                             window);

    // Add scrolled + button in box
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);

    // Add box to window
    gtk_container_add(GTK_CONTAINER(window), box);

    // Show all widgets
    gtk_widget_show_all(window);

    // Free pixbufs
    g_object_unref(pixbuf);
    g_object_unref(scaled);
}

/**
 * main:
 * Program entry point. Initializes and runs the GTK application.
 *
 * Parameters:
 *  - argc: argument count from the command line.
 *  - argv: argument vector from the command line.
 *
 * Returns:
 *  - int: application exit status returned by g_application_run().
 *
 * Behavior / Side effects:
 *  - Creates a new GtkApplication, connects the "activate" signal to
 *    on_activate, runs the application main loop, then releases the application
 *    object before returning the status code.
 *
 * Notes:
 *  - Command-line arguments (argc, argv) are forwarded to g_application_run().
 */
int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    // Create a new application
    app = gtk_application_new("com.example.GtkApplication", 0);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
