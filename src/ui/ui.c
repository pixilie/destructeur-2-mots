#include "../include/image/image.h"
#include "../include/ui.h"

#include <gtk/gtk.h>

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

/**
 * on_grayscale_clicked:
 * GTK callback invoked when the "Grayscale" button is clicked.
 *
 * Parameters:
 *  - button   : the clicked GtkButton (unused).
 *  - user_data: pointer to struct AppData.
 */
void on_grayscale_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    convert_to_grayscale(data->transformed);

    apply_transformations(data);

    printf("Image converted to grayscale\n");
}

/**
 * on_binarize_clicked:
 * GTK callback for the "Black & White" button.
 *
 * Parameters:
 *  - button   : the clicked GtkButton (unused).
 *  - user_data: pointer to struct AppData.
 */
void on_binarize_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    convert_to_grayscale(data->transformed);
    int threshold = convert_to_black_and_white(data->transformed);

    apply_transformations(data);

    printf("Image converted to black and white with threshold %i\n", threshold);
}

/**
 * on_rotate_clicked:
 * GTK callback for the "Rotate" button.
 *
 * Parameters:
 *  - button   : the clicked GtkButton (unused).
 *  - user_data: pointer to struct AppData.
 */
void on_rotate_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    double best_angle = detect_best_angle(data->transformed);
    data->rotation_angle = best_angle;

    apply_transformations(data);
    if (best_angle == 0)
    {
        printf("Image is already upright, no rotation applied !\n");
    }
    else
    {
        printf("Image automatically rotated by best rotation angle : %.2f "
               "degrees\n",
               best_angle);
    }
}

/**
 * on_draw_rectangle_clicked_clicked:
 * GTK callback for the "Draw Rectangle" button.
 *
 * Parameters:
 *  - button   : the clicked GtkButton (unused).
 *  - user_data: pointer to struct AppData.
 */
void on_draw_rectangle_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    int x1 = 200;
    int y1 = 200;
    int x2 = 400;
    int y2 = 300;
    int thickness = 5;
    
    draw_rectangle(data->transformed, x1, y1, x2, y2, thickness);
    
    apply_transformations(data);

    printf("Red rectangle drawn from (%i, %i) to (%i, %i) with thickness %i\n", x1, y1, x2, y2, thickness);
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

/**
 * on_activate:
 * GTK "activate" signal handler that builds the application UI and loads the
 * image.
 *
 * Parameters:
 *  - app      : the GtkApplication instance.
 *  - user_data: optional filename string (const char*) or NULL to use default.
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
    GtkWidget *draw_rectangle_button;
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

    // Create new buttons
    close_button = gtk_button_new_with_label("Fermer la fenêtre");
    g_signal_connect_swapped(close_button, "clicked",
                             G_CALLBACK(gtk_window_close), window);

    grayscale_button =
        gtk_button_new_with_label("Conversion en niveaux de gris");
    binarize_button = gtk_button_new_with_label("Conversion en noir et blanc");
    rotate_button = gtk_button_new_with_label("Rotation automatique");
    draw_rectangle_button = gtk_button_new_with_label("Dessiner un rectangle rouge");
    save_button = gtk_button_new_with_label("Sauvegarder l'image");
    reset_button = gtk_button_new_with_label("Réinitialiser l'image");

    gtk_box_pack_start(GTK_BOX(horizontal_box), grayscale_button, TRUE, TRUE,
                       5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), binarize_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), rotate_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), draw_rectangle_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), reset_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), save_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), close_button, TRUE, TRUE, 5);

    // Initialize AppData
    struct AppData *data = g_new(struct AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    data->save_index = 1;

    g_signal_connect(grayscale_button, "clicked",
                     G_CALLBACK(on_grayscale_clicked), data);
    g_signal_connect(binarize_button, "clicked",
                     G_CALLBACK(on_binarize_clicked), data);
    g_signal_connect(rotate_button, "clicked", G_CALLBACK(on_rotate_clicked),
                     data);
    g_signal_connect(draw_rectangle_button, "clicked", G_CALLBACK(on_draw_rectangle_clicked), data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), data);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked),
                     data);

    g_signal_connect(window, "destroy", G_CALLBACK(free_app_data), data);

    // Show all widgets
    gtk_widget_show_all(window);

    // Free pixbufs
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
