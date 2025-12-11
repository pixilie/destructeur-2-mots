/*
 * src/main.c
 *
 * Simple GTK application for loading, processing and saving images,
 * and managing a small neural network. Comments are short and follow the
 * project's style.
 */

#include "../include/dataset.h"
#include "../include/image/image.h"
#include "../include/neural_network.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * AppData:
 * Simple container for widgets and pixbufs used by callbacks.
 */
struct AppData
{
    GtkWidget *image;       /* image widget */
    GdkPixbuf *original;    /* original loaded pixbuf (owned) */
    GdkPixbuf *transformed; /* working copy (owned) */
    GdkPixbuf *current;     /* displayed pixbuf (owned) */
    double rotation_angle;
    int save_index;
};

NeuralNetwork *neural = NULL;
int treated = 0; /* 0 = not treated, 1 = treated */

/*
 * update_image:
 * Update the GtkImage widget with data->current and queue a redraw.
 */
void update_image(struct AppData *data)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), data->current);
    gtk_widget_queue_draw(data->image);
}

/*
 * apply_transformations:
 * Recompute data->current from data->transformed applying rotation.
 */
void apply_transformations(struct AppData *data)
{
    if (data->current)
        g_object_unref(data->current);

    if (data->rotation_angle != 0.0)
        data->current = rotate_image(data->transformed, data->rotation_angle);
    else
        data->current = gdk_pixbuf_copy(data->transformed);

    update_image(data);
}

/*
 * on_reset_clicked:
 * Restore the original image and reset transform state.
 */
void on_reset_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;

    treated = 0;

    if (!data || !data->original)
        return;

    if (data->current)
        g_object_unref(data->current);
    if (data->transformed)
        g_object_unref(data->transformed);

    data->rotation_angle = 0.0;
    data->transformed = gdk_pixbuf_copy(data->original);
    data->current = gdk_pixbuf_copy(data->original);
    update_image(data);
}

/*
 * on_save_clicked:
 * Show save dialog and write data->current as PNG.
 */
void on_save_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;
    if (!data || !data->current)
        return;

    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(data->image));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choose where to save", parent, GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            data->save_index++;
            GError *error = NULL;
            if (!gdk_pixbuf_save(data->current, filename, "png", &error, NULL))
            {
                g_printerr("Failed to save image: %s\n", error ? error->message : "unknown");
                if (error)
                    g_error_free(error);
            }
            else
            {
                g_print("Image saved as: %s\n", filename);
            }
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
}

/*
 * free_app_data:
 * Free GdkPixbufs and the AppData struct.
 */
void free_app_data(GtkWidget *widget __attribute__((unused)), gpointer user_data)
{
    struct AppData *data = user_data;
    if (!data)
        return;

    if (data->current)
        g_object_unref(data->current);
    if (data->original)
        g_object_unref(data->original);
    if (data->transformed)
        g_object_unref(data->transformed);

    g_free(data);
}

/*
 * pop_up_treated:
 * Show a small popup indicating the image was already treated.
 */
void pop_up_treated()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);
    GtkWidget *label = gtk_label_new("Image already treated");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_widget_show_all(window);
}

/*
 * automatic_treatement:
 * Apply grayscale, binarize, median filter and automatic rotation.
 */
void automatic_treatement(GtkButton *button, gpointer user_data)
{
    (void)button;
    struct AppData *data = user_data;
    if (!data || !data->transformed)
        return;

    if (treated == 0)
    {
        convert_to_grayscale(data->transformed);
        (void)convert_to_black_and_white(data->transformed); /* threshold not needed here */
        median_filter_3x3(data->transformed);
        data->transformed = rotate_image_automatic(data->transformed);

        apply_transformations(data);
        treated = 1;
    }
    else
    {
        pop_up_treated();
    }
}

/*
 * solver:
 * Placeholder for future solve action.
 */
void solver(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;
    /* Implement solution extraction and display here. */
}

/*
 * change_image:
 * Load image from filename and update AppData buffers.
 */
void change_image(const char *filename, gpointer user_data)
{
    struct AppData *data = (struct AppData *)user_data;
    treated = 0;

    if (!data)
    {
        g_printerr("Error: invalid AppData\n");
        return;
    }
    if (!data->image)
    {
        g_printerr("Error: image widget not initialized\n");
        return;
    }
    if (!filename || filename[0] == '\0')
    {
        g_printerr("Error: invalid image path\n");
        return;
    }

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (!pixbuf)
    {
        g_printerr("Failed to load image '%s': %s\n", filename, error ? error->message : "unknown");
        if (error)
            g_error_free(error);
        return;
    }

    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), pixbuf);

    if (data->original)
    {
        g_object_unref(data->original);
        data->original = NULL;
    }
    if (data->current)
    {
        g_object_unref(data->current);
        data->current = NULL;
    }
    if (data->transformed)
    {
        g_object_unref(data->transformed);
        data->transformed = NULL;
    }

    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    if (!data->current)
        g_printerr("Warning: failed to copy pixbuf for current\n");

    data->transformed = gdk_pixbuf_copy(pixbuf);
    if (!data->transformed)
        g_printerr("Warning: failed to copy pixbuf for transformed\n");

    data->rotation_angle = 0.0;
    data->save_index = 1;
}

/*
 * get_path_image:
 * Show file chooser and call change_image() on selection.
 */
void get_path_image(GtkWidget *widget, gpointer user_data)
{
    struct AppData *data = (struct AppData *)user_data;
    if (!data)
    {
        g_printerr("Error: invalid AppData in get_path_image\n");
        return;
    }

    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choose an image", parent, GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            change_image(filename, data);
            g_free(filename);
        }
        else
        {
            g_printerr("Error: no file selected\n");
        }
    }

    gtk_widget_destroy(dialog);
}

/*
 * load_neural:
 * Load neural network from file into global `neural`.
 */
void load_neural(const char *filename)
{
    if (!filename)
        return;

    g_print("Loading neural network from: %s\n", filename);
    neural = load_network(filename);
}

/*
 * get_neural_load_path:
 * Show a file chooser to select a neural network file and load it.
 */
void get_neural_load_path(GtkWidget *widget)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choose model file", parent, GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            load_neural(filename);
            g_free(filename);
        }
        else
        {
            g_printerr("Error: no file selected\n");
        }
    }
    gtk_widget_destroy(dialog);
}

/*
 * train_neural:
 * Create (if needed) and train a neural network using dataset.
 */
void train_neural(const char *filename)
{
    if (!filename)
        return;

    if (neural == NULL)
        neural = create_network(784, 128, 26);

    Dataset data = load_dataset(filename);
    train(neural, data.inputs, data.targets, data.samples, 0.01, 1000);
    g_print("Training started for dataset: %s\n", filename);
}

/*
 * get_neural_train_path:
 * Show folder chooser and start training on selected dataset.
 */
void get_neural_train_path(GtkWidget *widget)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choose dataset folder", parent, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            train_neural(filename);
            g_free(filename);
        }
        else
        {
            g_printerr("Error: no folder selected\n");
        }
    }

    gtk_widget_destroy(dialog);
}

/*
 * save_neural:
 * Show save dialog and serialize the current neural network.
 */
void save_neural(GtkWidget *widget)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choose where to save model", parent, GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);

    if (neural == NULL)
    {
        g_printerr("Error: cannot save, no neural network loaded\n");
        return;
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            save_network(neural, filename);
            g_print("Neural network saved to: %s\n", filename);
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
}

/*
 * on_activate:
 * Build UI, load initial image and connect callbacks.
 */
static void on_activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *vertical_box;
    GtkWidget *horizontal_box;
    GtkWidget *reset_button;
    GtkWidget *save_button;
    GtkWidget *close_button;
    GtkWidget *image;
    GtkWidget *scrolled;
    GtkWidget *treatement_button;
    GtkWidget *solver_button;
    GtkWidget *training_button;
    GtkWidget *load_button;
    GtkWidget *save_neural_button;
    GtkWidget *file_button;
    GtkWidget *center;
    GtkWidget *right_button;
    GtkWidget *description;
    GtkWidget *title_neural;

    char **filename = (char **)user_data;

    const char *image_path = get_image_path(*filename);
    if (!image_path)
    {
        g_printerr("Could not resolve image path for '%s'\n", *filename);
        return;
    }

    file_button = gtk_button_new_with_label("Load image");

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf)
    {
        g_printerr("Failed to load image '%s': %s\n", image_path, error ? error->message : "unknown");
        if (error)
            g_error_free(error);
        return;
    }

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Destructeur 2 mots");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vertical_box);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vertical_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *title = gtk_label_new("Destructeur 2 mots");
    gtk_box_pack_start(GTK_BOX(top_bar), title, FALSE, FALSE, 0);
    GtkWidget *space = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), space, TRUE, TRUE, 0);
    GtkWidget *space2 = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), space2, TRUE, TRUE, 0);

    close_button = gtk_button_new_with_label("Quit");
    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_close), window);
    gtk_box_pack_start(GTK_BOX(top_bar), close_button, TRUE, TRUE, 15);

    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, 900, 600, GDK_INTERP_BILINEAR);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    image = gtk_image_new_from_pixbuf(scaled);
    if (!gtk_image_get_pixbuf(GTK_IMAGE(image)))
    {
        g_printerr("ERROR: Image not loaded. Check image path\n");
        g_object_unref(scaled);
        return;
    }
    gtk_container_add(GTK_CONTAINER(scrolled), image);

    center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical_box), center, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(center), scrolled, TRUE, TRUE, 0);

    right_button = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(center), right_button, FALSE, FALSE, 0);

    horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_box, FALSE, FALSE, 5);

    treatement_button = gtk_button_new_with_label("Process");
    reset_button = gtk_button_new_with_label("Reset");
    save_button = gtk_button_new_with_label("Save");
    solver_button = gtk_button_new_with_label("Solve");

    description = gtk_label_new("Image:");

    gtk_box_pack_start(GTK_BOX(horizontal_box), treatement_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), solver_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), reset_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(horizontal_box), save_button, TRUE, TRUE, 5);

    struct AppData *data = g_new(struct AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    data->save_index = 1;

    title_neural = gtk_label_new("Neural network:");
    gtk_box_pack_start(GTK_BOX(right_button), title_neural, FALSE, FALSE, 0);

    training_button = gtk_button_new_with_label("Train and create network");
    g_signal_connect(training_button, "clicked", G_CALLBACK(get_neural_train_path), NULL);

    load_button = gtk_button_new_with_label("Load existing model");
    g_signal_connect(load_button, "clicked", G_CALLBACK(get_neural_load_path), NULL);

    save_neural_button = gtk_button_new_with_label("Save network");
    g_signal_connect(save_neural_button, "clicked", G_CALLBACK(save_neural), NULL);

    gtk_box_pack_start(GTK_BOX(right_button), file_button, FALSE, TRUE, 5);
    g_signal_connect(file_button, "clicked", G_CALLBACK(get_path_image), data);

    gtk_box_pack_start(GTK_BOX(right_button), training_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(right_button), load_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(right_button), save_neural_button, TRUE, TRUE, 5);

    g_signal_connect(treatement_button, "clicked", G_CALLBACK(automatic_treatement), data);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), data);
    g_signal_connect(solver_button, "clicked", G_CALLBACK(solver), data);

    gtk_widget_show_all(window);

    g_object_unref(scaled);

    /* free AppData when window is destroyed */
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(free_app_data), data);
}

/*
 * on_command_line:
 * Extract filename from command line and emit activate signal.
 */
int on_command_line(GApplication *app, GApplicationCommandLine *cmdline, gpointer user_data __attribute__((unused)))
{
    int argc;
    char **argv = g_application_command_line_get_arguments(cmdline, &argc);
    const char *filename = (argc > 1) ? argv[1] : "level_1_image_1.png";

    g_signal_emit_by_name(app, "activate", (gpointer)filename);

    g_strfreev(argv);
    return 0;
}

/*
 * main:
 * Program entry point.
 */
int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;
    char *filename;

    if (argc > 1)
        filename = g_strdup(argv[1]);
    else
        filename = g_strdup("level_1_image_1.png");

    app = gtk_application_new("com.example.GtkApplication", G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &filename);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    g_free(filename);

    return status;
}
