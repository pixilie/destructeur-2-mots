#include <stdlib.h>
#include "../include/image/image.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * AppData:
 * Small container for application state passed to callbacks.
 *
 * Fields:
 *  - image         : GTK image widget showing the current pixbuf.
 *  - original      : original loaded GdkPixbuf (keep to allow reset).
 *  - transformed   : working copy that receives in-place treatments.
 *  - current       : pixbuf currently shown (may be rotated view of
 * transformed).
 *  - rotation_angle: accumulated rotation angle in degrees.
 *  - save_index    : counter used to generate unique filenames when saving.
 */
struct AppData
{
    GtkWidget *image;
    GdkPixbuf *original;
    GdkPixbuf *transformed;
    GdkPixbuf *current;
    double rotation_angle;
    int save_index;
};

/**
 * update_image:
 * Update the GTK image widget to display data->current and request redraw.
 *
 * Parameters:
 *  - data: pointer to application state (must have data->image and
 * data->current).
 */
void update_image(struct AppData *data)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), data->current);
    gtk_widget_queue_draw(data->image);
}

/**
 * apply_transformations:
 * Recompute data->current from data->transformed applying the current rotation.
 *
 * Parameters:
 *  - data: pointer to application state.
 */
void apply_transformations(struct AppData *data)
{
    if (data->current)
    {
        g_object_unref(data->current);
    }
    if (data->rotation_angle != 0.0)
    {
        data->current = rotate_image(data->transformed, data->rotation_angle);
    }
    else
    {
        data->current = gdk_pixbuf_copy(data->transformed);
    }
    update_image(data);
}

/*
 * Grayscale 
 * Gtk callback invoked but not when button clicked, grayscale applied when call 
 * for the image treatement
 *
 * Parameters:
 *  - user_data: pointer to struct Appdata
 */
void greyscale(gpointer user_data)
{
    struct AppData *data = user_data;

    if(!data->transformed)
    {
	return;
    }

    convert_to_greyscale(data->transformed);

    apply_transformations(data);
    //see if we need to do all the details
    
    printf("Image converted to greyscale\n");
}

/*
 * Gtk callback for "White and Black"
 *
 * Parameters :
 *  -user_data : pointer to struct AppData
 */
void binarize_image(gpointer user_data)
{
    struct AppData *data = user_data;

    if(!data->transformed)
	    return;

    convert_to_greyscale(data->transformed);
    
    int treshold = convert_to_black_and_white(data->transformed);

    apply_transformations(data);

    printf("Image converted to black and white with treshold = %i\n", threshold);
}

/**
 * on_reset_clicked:
 * GTK callback for the "Reset" button.
 *
 * Parameters:
 *  - button   : the clicked GtkButton (unused).
 *  - user_data: pointer to struct AppData.
 */
void on_reset_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->original)
    {
        return;
    }

    if (data->current)
    {
        g_object_unref(data->current);
    }
    if (data->transformed)
    {
        g_object_unref(data->transformed);
    }

    data->rotation_angle = 0.0;

    data->transformed = gdk_pixbuf_copy(data->original);
    data->current = gdk_pixbuf_copy(data->original);
    update_image(data);
    printf("Image reset to original\n");
}

/**
 * on_save_clicked:
 * GTK callback for the "Save" button.
 *
 * Parameters:
 *  - button   : the clicked GtkButton (unused).
 *  - user_data: pointer to struct AppData.
 */
void on_save_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->current)
    {
        return;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "output%i.png", data->save_index);
    data->save_index++;

    GError *error = NULL;
    if (!gdk_pixbuf_save(data->current, filename, "png", &error, NULL))
    {
        g_printerr("Failed to save image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    printf("Image saved as %s\n", filename);
}

/**
 * free_app_data:
 * Cleanup handler called when the main window is destroyed.
 *
 * Parameters:
 *  - widget   : the widget that emitted the destroy signal (unused).
 *  - user_data: pointer to struct AppData to free.
 */
void free_app_data(GtkWidget *widget __attribute__((unused)),
                   gpointer user_data)
{
    struct AppData *data = user_data;
    if (data)
    {
        if (data->current)
        {
            g_object_unref(data->current);
        }
        if (data->original)
        {
            g_object_unref(data->original);
        }
        if (data->transformed)
        {
            g_object_unref(data->transformed);
        }
        g_free(data);
    }
}

/*
 * Automatic treatment:
 * Applie the automatic treatement with these step :
 *  - Greyscale
 *  - Binarize
 *  - Median filter
 *  - Automatic rotation
 */
void automatic_treatement(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    greyscale(data);
    binarize_image(data);
    data->transformed = median_filter_3x3(data->transformed);
    data->transformed = rotate_image_automatic(data->transformed);

    apply_transformation(data);
}

/*
 * Solver:
 * Call the solver function when button clicked and then display
 * the solution 
 *
 * Parameters :
 * - button    : the clicked button (unused)
 * - user_data : pointer to struct AppData
 */
void solver(GtkButton *button, gpointer user_data)
{
    (void)button;
}


/*
 * on_activate:
 * Gtk "activate" signal handler that builds the app UI and loads
 * the image.
 *
 * Parameters :
 *  - app       : the GtkApplication instance
 *  - user_data : optional filename string (const char*) or NULL to use default
 */
static void on_activate(GtkApplication *app, gpointer user_data)
{
    // UI Variables
    GtkWidget *window;
    GtkWidget *vertical_box;
    GtkWidget *horizontal_box;
    GtkWidget *grayscale_button;
    GtkWidget *binarize_button;
    GtkWidget *rotate_button;
    GtkWidget *reset_button;
    GtkWidget *save_button;
    GtkWidget *close_button;
    GtkWidget *image;
    GtkWidget *scrolled;

   // Load image
    char *filename = (char *)user_data;
    if (!filename)
    {
        filename = "level_1_image_1.png";
    }
    const char *image_path = get_image_path(filename);
    if (!image_path)
    {
        g_printerr("Could not resolve image path for '%s'\n", filename);
        return;
    }
    printf("Loading image at: %s\n", image_path);

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf)
    {
        g_printerr("Failed to load image '%s': %s\n", image_path,
                   error->message);
        g_error_free(error);
        return;
    }

   // Create a new window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Image");
    gtk_window_set_default_size(GTK_WINDOW(window), 2000, 1000);

    // Create a vertical box
    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vertical_box);

    GdkPixbuf *scaled =
        gdk_pixbuf_scale_simple(pixbuf, 1000, 700, GDK_INTERP_BILINEAR);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vertical_box), scrolled, TRUE, TRUE, 5);

    image = gtk_image_new_from_pixbuf(scaled);
    if (!gtk_image_get_pixbuf(GTK_IMAGE(image)))
    {
        g_print("ERROR: Image not loaded. Check image path\n");
        return;
    }
    gtk_container_add(GTK_CONTAINER(scrolled), image);

    horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_box, FALSE, FALSE, 5);


   //Create new buttons
   close_button = gtk_button_new_with_label("Quitter");
   g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_close), window);

   save_button = gtk_button_new_with_label("Sauvegarde");
   reset_button = gtk_button_new_with_label("Réinitialiser");
   treatement_button = gtk_button_new_with_label("Traiter");
   solver_button = gtk_button_new_with_label("Resoudre");

   gtk_box_pack_start(GTK_BOX(horizontal_box), close_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), save_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), reset_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), solver_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), treatement_button, TRUE, TRUE, 5);

   // Initialize AppData
    struct AppData *data = g_new(struct AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    data->save_index = 1;

    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), data);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), data);
    g_signal_connect(treatement_button, "clicked", G_CALLBACK(automatic_treatement), data);
    g_signal_connect(solver_button, "clicked", G_CALLBACK(solver), data);

    //Show all widgets
    gtk_widget_show_all(window);

    //free pixbufs
    g_object_unref(scaled);
}


/**
 * on_command_line:
 * GApplication "command-line" handler that extracts the filename argument and
 * forwards it to the activate handler.
 *
 * Parameters:
 *  - app      : GApplication instance.
 *  - cmdline  : GApplicationCommandLine containing argc/argv.
 *  - user_data: unused.
 *
 * Returns:
 *  - int: status code (0).
 */
int on_command_line(GApplication *app, GApplicationCommandLine *cmdline,
                    gpointer user_data __attribute__((unused)))
{
    int argc;
    char **argv = g_application_command_line_get_arguments(cmdline, &argc);
    const char *filename;
    if (argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        filename = "level_1_image_1.png";
    }

    g_signal_emit_by_name(app, "activate", (gpointer)filename);

    g_strfreev(argv);
    return 0;
}

/**
 * main:
 * Program entry point. Creates a GtkApplication that handles command-line
 * arguments and runs the main loop.
 *
 * Parameters:
 *  - argc, argv: standard program arguments. Optional first argument is the
 *                image filename to load at startup.
 *
 * Returns:
 *  - int: application exit status.
 */
int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;
    char *filename;
    if (argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        filename = "level_1_image_1.png";
    }

    // Create a new application
    app = gtk_application_new("com.example.GtkApplication",
                              G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), filename);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}


//int main() { return EXIT_SUCCESS; }
