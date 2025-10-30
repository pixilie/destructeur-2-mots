#include "../include/image_helpers.h"
#include "../include/image_rotation.h"
#include "../include/image_treatment.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct AppData
{
    GtkWidget *image;
    GdkPixbuf *original;
    GdkPixbuf *transformed; //all non-rotation transformations
    GdkPixbuf *current;
    double rotation_angle;
};

void update_image(struct AppData *data)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), data->current);
    gtk_widget_queue_draw(data->image);
}

void on_grayscale_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if(!data->transformed)
    {
        return;
    }

    convert_to_grayscale(data->transformed);

    if(data->current)
    {
        g_object_unref(data->current);
    }

    data->current = gdk_pixbuf_copy(data->transformed);

    if(data->rotation_angle != 0.0) 
    {
        GdkPixbuf *rotated = rotate_image(data->transformed, data->rotation_angle);
        g_object_unref(data->current);
        data->current = rotated;
    }

    update_image(data);
}

void on_binarize_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if(!data->transformed)
    {
        return;
    }
    
    convert_to_grayscale(data->transformed);
    binarize_image(data->transformed, 180);

    if(data->current)
    {
        g_object_unref(data->current);
    }

    data->current = gdk_pixbuf_copy(data->transformed);

    if(data->rotation_angle != 0.0) 
    {
        GdkPixbuf *rotated = rotate_image(data->transformed, data->rotation_angle);
        g_object_unref(data->current);
        data->current = rotated;
    }
    update_image(data);
}

void on_rotate_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if(!data->transformed)
    {
        return;
    }
    data->rotation_angle += 45.0;
    if(data->rotation_angle >= 360.0)
    {
        data->rotation_angle -= 360.0;
    }
    GdkPixbuf *rotated = rotate_image(data->transformed, data->rotation_angle);

    if(data->current)
    {
        g_object_unref(data->current);
    }

    data->current = rotated;

    update_image(data);
}

void on_reset_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if(!data->original)
    {    
        return;
    }

    if(data->current)
    {
        g_object_unref(data->current);
    }
    if(data->transformed)
    {
        g_object_unref(data->transformed);
    }

    data->rotation_angle = 0.0;

    data->transformed = gdk_pixbuf_copy(data->original);
    data->current = gdk_pixbuf_copy(data->original);
    update_image(data);
}


void free_app_data(GtkWidget *widget __attribute__((unused)), gpointer user_data)
{
    struct AppData *data = user_data;
    if(data)
    {
        if(data->current)
        {
            g_object_unref(data->current);
        }
        if(data->original)
        {
            g_object_unref(data->original);
        }
        if(data->transformed)
        {
            g_object_unref(data->transformed);
        }
        g_free(data);
    }
}

static void on_activate(GtkApplication *app, gpointer user_data) __attribute__((unused));

static void on_activate(GtkApplication *app, gpointer user_data)
{
    // UI Variables
    GtkWidget *window;
    GtkWidget *vertical_box;
    GtkWidget *horizontal_box;
    GtkWidget *close_button;
    GtkWidget *grayscale_button;
    GtkWidget *binarize_button;
    GtkWidget *rotate_button;
    GtkWidget *reset_button;
    GtkWidget *image;
    GtkWidget *scrolled;

    //Load image
    char *filename = (char *) user_data;
    if(!filename)
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
	g_printerr("Failed to load image '%s': %s\n", image_path, error->message);
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

    // Create new buttons
    close_button = gtk_button_new_with_label("Fermer la fenêtre");
    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_close),
                             window);

    grayscale_button = gtk_button_new_with_label("Grayscale");
    binarize_button = gtk_button_new_with_label("Black & White");
    rotate_button = gtk_button_new_with_label("Rotate");
    reset_button = gtk_button_new_with_label("Réinitialiser l'image");

    gtk_box_pack_start(GTK_BOX(horizontal_box), grayscale_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), binarize_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), rotate_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), reset_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), close_button, TRUE, TRUE, 5);
    
    //Initialize AppData
    struct AppData *data = g_new(struct AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    
    g_signal_connect(grayscale_button, "clicked", G_CALLBACK(on_grayscale_clicked), data);
    g_signal_connect(binarize_button, "clicked", G_CALLBACK(on_binarize_clicked), data);
    g_signal_connect(rotate_button, "clicked", G_CALLBACK(on_rotate_clicked), data);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), data);

    g_signal_connect(window, "destroy", G_CALLBACK(free_app_data), data);

    // Show all widgets
    gtk_widget_show_all(window);

    // Free pixbufs
    g_object_unref(scaled);
}

int on_command_line(GApplication *app, GApplicationCommandLine *cmdline, gpointer user_data __attribute__((unused)))
{
    int argc;
    char **argv = g_application_command_line_get_arguments(cmdline, &argc);
    const char *filename;
    if(argc > 1)
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


int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;
    char *filename;
    if(argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        filename = "level_1_image_1.png";
    }
   
    // Create a new application
    app = gtk_application_new("com.example.GtkApplication", G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), filename);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
