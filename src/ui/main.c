#include "../include/dataset.h"
#include "../include/image_processing/image_processing.h"
#include "../include/neural_network.h"
#include "../include/ui/ui.h"

#include <fontconfig/fontconfig.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_MODEL_PATH "assets/ocr_model"

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

NeuralNetwork *neural = NULL;
int is_processed = 0;

/*
 * update_image:
 * Update the GtkImage widget with data->current and queue a redraw.
 */
void update_image(AppData *data)
{
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), data->current);
    gtk_widget_queue_draw(data->image);
}

/*
 * apply_transformations:
 * Recompute data->current from data->transformed applying rotation.
 */
void apply_transformations(AppData *data)
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
 * free_app_data:
 * Free GdkPixbufs and the AppData struct.
 */
void free_app_data(GtkWidget *widget __attribute__((unused)),
                   gpointer user_data)
{
    AppData *data = user_data;
    if (!data)
        return;

    if (data->current)
        g_object_unref(data->current);
    if (data->original)
        g_object_unref(data->original);
    if (data->transformed)
        g_object_unref(data->transformed);

    free_pipeline(data->pipelineResult);
    g_free(data);
}

/*
 * on_activate:
 * Build UI, load initial image and connect callbacks.
 */
static void on_activate(GtkApplication *app, gpointer user_data)
{
    load_css();

    GtkWidget *window;
    GtkWidget *vertical_box;
    GtkWidget *horizontal_center_box;
    GtkWidget *image_container_box;
    GtkWidget *sidebar_box;
    GtkWidget *bottom_box;

    GtkWidget *logo_image;
    GtkWidget *title;
    GtkWidget *close_button;
    GtkWidget *image;
    GtkWidget *scrolled;

    GtkWidget *lbl_neural;
    GtkWidget *btn_load_img_sidebar;
    GtkWidget *btn_training;
    GtkWidget *btn_load_neural;
    GtkWidget *btn_save_neural;

    GtkWidget *lbl_image;
    GtkWidget *btn_process;
    GtkWidget *btn_solve;
    GtkWidget *btn_reset;
    GtkWidget *btn_save_image;

    char **filename = (char **)user_data;

    const char *image_path = get_image_path(*filename);
    if (!image_path)
    {
        g_printerr("Could not resolve image path for '%s'\n", *filename);
        return;
    }

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf)
    {
        g_printerr("Failed to load image '%s': %s\n", image_path,
                   error ? error->message : "unknown");
        if (error)
            g_error_free(error);
        return;
    }

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Destructeur 2 mots");
    gtk_window_set_default_size(GTK_WINDOW(window), 1300, 900);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vertical_box);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vertical_box), top_bar, FALSE, FALSE, 0);

    char *exe_dir = get_executable_dir();
    char logo_path[1024];
    snprintf(logo_path, sizeof(logo_path), "%s/../assets/logo.png", exe_dir);

    GdkPixbuf *logo_pixbuf = gdk_pixbuf_new_from_file(logo_path, &error);
    if (logo_pixbuf)
    {
        GdkPixbuf *scaled_logo =
            gdk_pixbuf_scale_simple(logo_pixbuf, 70, 70, GDK_INTERP_BILINEAR);
        logo_image = gtk_image_new_from_pixbuf(scaled_logo);
        gtk_box_pack_start(GTK_BOX(top_bar), logo_image, FALSE, FALSE, 5);
        g_object_unref(logo_pixbuf);
        g_object_unref(scaled_logo);
    }
    else
    {
        g_printerr("Failed to load logo: %s\n",
                   error ? error->message : "unknown");
        if (error)
            g_error_free(error);
    }

    title = gtk_label_new("Destructeur 2 Mots");
    gtk_widget_set_name(title, "app-title");
    gtk_box_pack_start(GTK_BOX(top_bar), title, FALSE, FALSE, 0);

    GtkWidget *space = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), space, TRUE, TRUE, 0);

    close_button = gtk_button_new_with_label("Quitter");
    GtkStyleContext *ctx = gtk_widget_get_style_context(close_button);
    gtk_style_context_add_class(ctx, "quit-btn");
    g_signal_connect_swapped(close_button, "clicked",
                             G_CALLBACK(gtk_window_close), window);
    gtk_box_pack_start(GTK_BOX(top_bar), close_button, FALSE, FALSE, 0);

    GdkPixbuf *scaled =
        gdk_pixbuf_scale_simple(pixbuf, 1000, 700, GDK_INTERP_BILINEAR);

    horizontal_center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_center_box, TRUE, TRUE,
                       0);

    image_container_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(image_container_box, "image-border");
    gtk_box_pack_start(GTK_BOX(horizontal_center_box), image_container_box,
                       TRUE, TRUE, 0);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(image_container_box), scrolled, TRUE, TRUE, 5);

    image = gtk_image_new_from_pixbuf(scaled);
    gtk_container_add(GTK_CONTAINER(scrolled), image);

    sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_size_request(sidebar_box, 300, -1);
    gtk_box_pack_start(GTK_BOX(horizontal_center_box), sidebar_box, FALSE,
                       FALSE, 0);

    lbl_neural = gtk_label_new("Réseau de neurones");
    ctx = gtk_widget_get_style_context(lbl_neural);
    gtk_style_context_add_class(ctx, "bold-label");
    gtk_box_pack_start(GTK_BOX(sidebar_box), lbl_neural, FALSE, FALSE, 5);

    AppData *data = g_new0(AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    data->save_index = 1;
    data->pipelineResult = load_pipeline(*filename, neural);

    btn_load_img_sidebar = gtk_button_new_with_label("Charger une image");
    gtk_box_pack_start(GTK_BOX(sidebar_box), btn_load_img_sidebar, FALSE, FALSE,
                       5);
    g_signal_connect(btn_load_img_sidebar, "clicked",
                     G_CALLBACK(get_path_image), data);

    btn_training = gtk_button_new_with_label("Entrainement");
    gtk_box_pack_start(GTK_BOX(sidebar_box), btn_training, FALSE, FALSE, 5);
    g_signal_connect(btn_training, "clicked", G_CALLBACK(get_neural_train_path),
                     NULL);

    btn_load_neural = gtk_button_new_with_label("Charger");
    gtk_box_pack_start(GTK_BOX(sidebar_box), btn_load_neural, FALSE, FALSE, 5);
    g_signal_connect(btn_load_neural, "clicked",
                     G_CALLBACK(get_neural_load_path), NULL);

    btn_save_neural = gtk_button_new_with_label("Sauvegarder");
    gtk_box_pack_start(GTK_BOX(sidebar_box), btn_save_neural, FALSE, FALSE, 5);
    g_signal_connect(btn_save_neural, "clicked", G_CALLBACK(save_neural), NULL);

    bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_pack_start(GTK_BOX(vertical_box), bottom_box, FALSE, FALSE, 10);

    lbl_image = gtk_label_new("Image :");
    ctx = gtk_widget_get_style_context(lbl_image);
    gtk_style_context_add_class(ctx, "bold-label");
    gtk_box_pack_start(GTK_BOX(bottom_box), lbl_image, FALSE, FALSE, 10);

    btn_process = gtk_button_new_with_label("Traiter");
    gtk_box_pack_start(GTK_BOX(bottom_box), btn_process, TRUE, TRUE, 0);
    g_signal_connect(btn_process, "clicked", G_CALLBACK(automatic_treatement),
                     data);

    btn_solve = gtk_button_new_with_label("Résoudre");
    gtk_box_pack_start(GTK_BOX(bottom_box), btn_solve, TRUE, TRUE, 0);
    g_signal_connect(btn_solve, "clicked", G_CALLBACK(solver), data);

    btn_reset = gtk_button_new_with_label("Réinitialiser");
    gtk_box_pack_start(GTK_BOX(bottom_box), btn_reset, TRUE, TRUE, 0);
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset_clicked), data);

    btn_save_image = gtk_button_new_with_label("Sauvegarder");
    gtk_box_pack_start(GTK_BOX(bottom_box), btn_save_image, TRUE, TRUE, 0);
    g_signal_connect(btn_save_image, "clicked", G_CALLBACK(on_save_clicked),
                     data);

    gtk_widget_show_all(window);

    g_object_unref(scaled);

    // Free AppData and pipelineResult when window is destroyed
    g_signal_connect(window, "destroy", G_CALLBACK(free_app_data), data);
}

/*
 * on_command_line:
 * Extract filename from command line and emit activate signal.
 */
int on_command_line(GApplication *app, GApplicationCommandLine *cmdline,
                    gpointer user_data __attribute__((unused)))
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

    char neural_path[1024];
    char *exe_dir = get_executable_dir();
    snprintf(neural_path, sizeof(neural_path), "%s/../%s", exe_dir,
             DEFAULT_MODEL_PATH);
    printf("Default neural netwotk path loaded: %s\n", neural_path);
    neural = load_network(neural_path);

    if (argc > 1)
        filename = g_strdup(argv[1]);
    else
        filename = g_strdup("level_1_image_1.png");

    app = gtk_application_new("com.example.GtkApplication",
                              G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &filename);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    g_free(filename);
    //FcFini();

    return status;
}
