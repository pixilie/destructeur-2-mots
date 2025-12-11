#define _POSIX_C_SOURCE 200809L

#include "../include/ui.h"
#include "../include/image/image.h"
#include "line_detection.h"

#include <gtk/gtk.h>

#include <libgen.h>
#include <unistd.h>

/*
 * update_image:
 * Set GTK image widget from data->current and queue a redraw.
 *
 * Parameters:
 *  - data: AppData* containing the widget and current pixbuf.
 */
void update_image(AppData *data)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), data->current);
    gtk_widget_queue_draw(data->image);
}

/*
 * apply_transformations:
 * Recompute data->current from data->transformed and apply rotation if any.
 *
 * Parameters:
 *  - data: AppData* holding transformed image and rotation angle.
 */
void apply_transformations(AppData *data)
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
 * on_grayscale_clicked:
 * GTK callback: convert the current image to grayscale and update UI.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_grayscale_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    convert_to_grayscale(data->transformed);

    apply_transformations(data);

    printf("Image converted to grayscale\n");
}

/*
 * on_convert_to_black_and_white_clicked:
 * GTK callback: convert image to black & white (binarize) and update UI.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_convert_to_black_and_white_clicked(GtkButton *button,
                                           gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    convert_to_grayscale(data->transformed);
    int threshold = convert_to_black_and_white(data->transformed);

    apply_transformations(data);

    printf("Image converted to black and white with threshold %i\n", threshold);
}

/*
 * on_filter_clicked:
 * GTK callback: apply median filter to the transformed image and refresh.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_filter_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    median_filter_3x3(data->transformed);

    apply_transformations(data);

    printf("Image filtered with a median filter\n");
}

/*
 * on_rotate_clicked:
 * GTK callback: detect best rotation angle and apply it to the image.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_rotate_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

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

/*
 * on_draw_rectangle_clicked_clicked:
 * GTK callback: draw rectangles around solved words on the transformed image.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_draw_rectangle_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    Words words = data->pipelineResult.words;
    if (!words.solved_words_image_coos)
    {
        printf("No solved words image coordinates found to draw around solved "
               "words !\n");
        return;
    }

    for (int i = 0; i < words.detected_words_count; i++)
    {
        int x1 = words.solved_words_image_coos[i][0];
        int y1 = words.solved_words_image_coos[i][1];
        int x2 = words.solved_words_image_coos[i][2];
        int y2 = words.solved_words_image_coos[i][3];
        int x3 = words.solved_words_image_coos[i][4];
        int y3 = words.solved_words_image_coos[i][5];
        int x4 = words.solved_words_image_coos[i][6];
        int y4 = words.solved_words_image_coos[i][7];

        if (x1 > 0 && y1 > 0 && x2 > 0 && y2 > 0 && x3 > 0 && y3 > 0 &&
            x4 > 0 && y4 > 0)
        {
            draw_rectangle(data->transformed, x1, y1, x2, y2, x3, y3, x4, y4,
                           5);
            printf("Red rectangle drawn at (%i, %i) (%i, %i) (%i, %i) (%i, %i) "
                   "with thickness %i\n",
                   x1, y1, x2, y2, x3, y3, x4, y4, 5);
        }
    }

    /*

    // Straight rectangle
    int x1 = 200, y1 = 100;
    int x2 = 300, y2 = 100;
    int x3 = 300, y3 = 300;
    int x4 = 200, y4 = 300;

    int thickness = 5;

    draw_rectangle(data->transformed, x1, y1, x2, y2, x3, y3, x4, y4,
                   thickness);
    printf("Red rectangle drawn at (%i, %i) (%i, %i) (%i, %i) (%i, %i) with "
           "thickness %i\n",
           x1, y1, x2, y2, x3, y3, x4, y4, thickness);

    // Diagonal rectangle
    x1 = 500, y1 = 300;
    x2 = 600, y2 = 300;
    x3 = 700, y3 = 400;
    x4 = 600, y4 = 400;

    draw_rectangle(data->transformed, x1, y1, x2, y2, x3, y3, x4, y4,
                   thickness);

    */

    apply_transformations(data);

    /*

    printf("Red rectangle drawn at (%i, %i) (%i, %i) (%i, %i) (%i, %i) with "
           "thickness %i\n",
           x1, y1, x2, y2, x3, y3, x4, y4, thickness);

    */
}

/*
 * on_reset_clicked:
 * GTK callback: restore the original image and reset transformations.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_reset_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

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

/*
 * on_save_clicked:
 * GTK callback: save the currently displayed image to a PNG file.
 *
 * Parameters:
 *  - button: GtkButton* (unused).
 *  - user_data: AppData* pointer.
 */
void on_save_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

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

/*
 * free_app_data:
 * Cleanup handler for application data; unref pixbufs and free AppData.
 *
 * Parameters:
 *  - widget: GtkWidget* (unused).
 *  - user_data: AppData* pointer to free.
 */
void free_app_data(GtkWidget *widget __attribute__((unused)),
                   gpointer user_data)
{
    AppData *data = user_data;
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
 * on_activate:
 * GTK activate handler: construct UI, load image and initialize AppData.
 *
 * Parameters:
 *  - app: GtkApplication* instance.
 *  - user_data: optional filename (const char*) or NULL for default.
 */
static void on_activate(GtkApplication *app, gpointer user_data)
{
    // UI Variables
    GtkWidget *window;
    GtkWidget *vertical_box;
    GtkWidget *horizontal_box;

    GtkWidget *grayscale_button;
    GtkWidget *convert_to_black_and_white_button;
    GtkWidget *filter_button;
    GtkWidget *rotate_button;
    GtkWidget *draw_rectangle_button;
    GtkWidget *reset_button;
    GtkWidget *save_button;
    GtkWidget *close_button;

    GtkWidget *image;
    GtkWidget *scrolled;

    char *exe_dir = get_executable_dir();

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
    convert_to_black_and_white_button =
        gtk_button_new_with_label("Conversion en noir et blanc");
    filter_button = gtk_button_new_with_label("Filtrer l'image");
    rotate_button = gtk_button_new_with_label("Rotation automatique");
    draw_rectangle_button =
        gtk_button_new_with_label("Dessiner un rectangle rouge");
    save_button = gtk_button_new_with_label("Sauvegarder l'image");
    reset_button = gtk_button_new_with_label("Réinitialiser l'image");

    gtk_box_pack_start(GTK_BOX(horizontal_box), grayscale_button, TRUE, TRUE,
                       5);
    gtk_box_pack_start(GTK_BOX(horizontal_box),
                       convert_to_black_and_white_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), filter_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), rotate_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), draw_rectangle_button, TRUE,
                       TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), reset_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), save_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), close_button, TRUE, TRUE, 5);

    // Initialize AppData
    AppData *data = g_new(AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    data->save_index = 1;

    char grid_path[512];
    char letters_path[512];
    snprintf(grid_path, sizeof(grid_path), "%s/tests/results/ui_output/grid",
             exe_dir);
    snprintf(letters_path, sizeof(letters_path),
             "%s/tests/results/ui_output/letters", exe_dir);

    data->pipelineResult = pipeline(filename, grid_path, letters_path);

    g_signal_connect(grayscale_button, "clicked",
                     G_CALLBACK(on_grayscale_clicked), data);
    g_signal_connect(convert_to_black_and_white_button, "clicked",
                     G_CALLBACK(on_convert_to_black_and_white_clicked), data);
    g_signal_connect(filter_button, "clicked", G_CALLBACK(on_filter_clicked),
                     data);
    g_signal_connect(rotate_button, "clicked", G_CALLBACK(on_rotate_clicked),
                     data);
    g_signal_connect(draw_rectangle_button, "clicked",
                     G_CALLBACK(on_draw_rectangle_clicked), data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), data);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked),
                     data);

    g_signal_connect(window, "destroy", G_CALLBACK(free_app_data), data);

    // Show all widgets
    gtk_widget_show_all(window);

    // Free pixbufs
    g_object_unref(scaled);
}

/*
 * on_command_line:
 * GApplication command-line handler: extract optional filename and trigger activate.
 *
 * Parameters:
 *  - app: GApplication* instance.
 *  - cmdline: GApplicationCommandLine* containing argc/argv.
 *  - user_data: unused.
 *
 * Returns:
 *  - int exit status (0).
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

/*
 * main:
 * Program entry: create GtkApplication, connect handlers and run the loop.
 *
 * Parameters:
 *  - argc, argv: standard program arguments; optional first arg is image name.
 *
 * Returns:
 *  - int application exit status.
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
