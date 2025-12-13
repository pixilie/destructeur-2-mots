#include "../include/dataset.h"
#include "../include/grid_cutting.h"
#include "../include/image/image.h"
#include "../include/neural_network.h"
#include "../include/ui.h"
#include "image/image_helpers.h"

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
int is_processed = 0; // 0 = not processed, 1 = processed

void load_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(
        provider,
        "window { background-color: white; }"
        "#app-title { font-size: 24px; font-weight: bold; margin: 10px; }"
        ".quit-btn { color: red; background: white; border: 2px solid red; "
        "border-radius: 5px; font-weight: bold; margin: 10px; }"
        ".quit-btn:hover { background: #ffe6e6; }"
        "#image-border { border: 4px solid #4CAF50; border-radius: 8px; "
        "margin: 10px; background-color: white; }"
        ".bold-label { font-weight: bold; font-size: 14px; margin-bottom: 5px; "
        "}"
        "button { background: white; border: 2px solid black; border-radius: "
        "8px; padding: 5px 10px; font-weight: bold; margin: 5px; }"
        "button:hover { background: #f0f0f0; }",
        -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

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
 * update_progress_ui:
 * Update the progress bar.
 */
void update_progress_ui(int current_epoch, int total_epochs, void *user_data)
{
    GtkProgressBar *bar = GTK_PROGRESS_BAR(user_data);

    double fraction = (double)current_epoch / total_epochs;
    gtk_progress_bar_set_fraction(bar, fraction);

    char text[32];
    snprintf(text, sizeof(text), "Epoch %d/%d", current_epoch, total_epochs);
    gtk_progress_bar_set_text(bar, text);

    while (gtk_events_pending())
    {
        gtk_main_iteration();
    }
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
 * on_reset_clicked:
 * Restore the original image and reset transform state.
 */
void on_reset_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    AppData *data = user_data;

    is_processed = 0;

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
    AppData *data = user_data;
    if (!data || !data->current)
        return;

    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(data->image));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choisir l'emplacement où enregistrer l'image", parent,
        GTK_FILE_CHOOSER_ACTION_SAVE, "_Annuler", GTK_RESPONSE_CANCEL,
        "_Enregistrer", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            data->save_index++;
            GError *error = NULL;
            if (!gdk_pixbuf_save(data->current, filename, "png", &error, NULL))
            {
                g_printerr("Failed to save image: %s\n",
                           error ? error->message : "unknown");
                if (error)
                    g_error_free(error);
            }
            else
            {
                printf(COLOR_GREEN "[SUCCES] " COLOR_RESET
                                   "Image sauvegardée dans : %s\n",
                       filename);
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
 * pop_up_treated:
 * Show a small popup indicating the image was already treated.
 */
void pop_up_treated()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);
    GtkWidget *label = gtk_label_new("L'image a déja été traitée");
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
    AppData *data = user_data;
    if (!data || !data->transformed)
        return;

    if (is_processed == 0)
    {
        convert_to_grayscale(data->transformed);
        median_filter_3x3(data->transformed);
        (void)convert_to_black_and_white(data->transformed);
        GdkPixbuf *rotated = rotate_image_automatic(data->transformed);
        // g_object_unref(data->transformed);
        data->transformed = rotated;

        apply_transformations(data);
        is_processed = 1;
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
    AppData *data = user_data;

    if (!data->transformed)
    {
        return;
    }

    double rotation_angle = data->pipelineResult->rotation_angle;
    if (rotation_angle != 0.0)
    {
        GdkPixbuf *rotated = rotate_image(data->transformed, rotation_angle);
        g_object_unref(data->transformed);
        data->transformed = rotated;
        data->rotation_angle = rotation_angle;
    }

    apply_transformations(data);

    Words *words = &data->pipelineResult->words;
    if (!words->solved_words_image_coos)
    {
        printf(COLOR_RED "[ERREUR] " COLOR_RESET
                         "Les coordonnées des mots n'ont pas été trouvées dans "
                         "le solver pour dessiner autour des mots!\n");
        return;
    }

    if (words->detected_words_count == 0)
    {
        printf(COLOR_RED "[ERREUR] " COLOR_RESET
                         "Aucun mot n'a été détecté dans la grille!\n");
        return;
    }

    int thickness = 3;
    int is_drawn = 0;

    for (int i = 0; i < words->detected_words_count; i++)
    {
        int x1 = words->solved_words_image_coos[i][0];
        int y1 = words->solved_words_image_coos[i][1];
        int x2 = words->solved_words_image_coos[i][2];
        int y2 = words->solved_words_image_coos[i][3];
        int x3 = words->solved_words_image_coos[i][4];
        int y3 = words->solved_words_image_coos[i][5];
        int x4 = words->solved_words_image_coos[i][6];
        int y4 = words->solved_words_image_coos[i][7];

        if (x1 > 0 && y1 > 0 && x2 > 0 && y2 > 0 && x3 > 0 && y3 > 0 &&
            x4 > 0 && y4 > 0)
        {
            draw_rectangle(data->transformed, x1, y1, x2, y2, x3, y3, x4, y4,
                           thickness);
            is_drawn = 1;
            printf("Rectangle drawn at (%i, %i) (%i, %i) (%i, %i) (%i, %i) "
                   "with thickness %i\n",
                   x1, y1, x2, y2, x3, y3, x4, y4, thickness);
        }
    }

    if (is_drawn == 0)
    {
        printf(COLOR_RED
               "[ERREUR] " COLOR_RESET
               "Aucun mot n'a été marqué comme résolu dans la grille\n");
    }

    data->rotation_angle = 0;
    apply_transformations(data);
    data->rotation_angle = rotation_angle;
}

PipelineResult *load_pipeline(char *filename, NeuralNetwork *nn)
{
    char *exe_dir = get_executable_dir();
    char grid_path[512];
    char letters_path[512];
    snprintf(grid_path, sizeof(grid_path), "%s/tests/results/ui_output/grid",
             exe_dir);
    snprintf(letters_path, sizeof(letters_path),
             "%s/tests/results/ui_output/letters", exe_dir);

    return pipeline(filename, nn);
}

/*
 * change_image:
 * Load image from filename and update AppData buffers.
 */
void change_image(const char *filename, gpointer user_data)
{
    AppData *data = user_data;
    is_processed = 0;

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
        g_printerr("Failed to load image '%s': %s\n", filename,
                   error ? error->message : "unknown");
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
    is_processed = 0;

    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), data->current);
    gtk_widget_queue_draw(data->image);

    // Free old pipelineResult fully
    free_pipeline(data->pipelineResult);
    data->pipelineResult = NULL;

    printf(COLOR_YELLOW "[APP] " COLOR_RESET "Image changée: %s\n", filename);

    data->pipelineResult = load_pipeline((char *)filename, neural);
}

/*
 * get_path_image:
 * Show file chooser and call change_image() on selection.
 */
void get_path_image(GtkWidget *widget, gpointer user_data)
{
    AppData *data = user_data;
    if (!data)
    {
        g_printerr("Error: invalid AppData in get_path_image\n");
        return;
    }

    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choisir une nouvelle image à charger", parent,
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Annuler", GTK_RESPONSE_CANCEL,
        "_Ouvrir", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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

    neural = load_network(filename);
    if (neural)
    {
        printf(COLOR_YELLOW "[INFO] " COLOR_RESET
                            "Modèle du réseau de neurones chargé : %s\n",
               filename);
    }
}

/*
 * get_neural_load_path:
 * Show a file chooser to select a neural network file and load it.
 */
void get_neural_load_path(GtkWidget *widget)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choisir le fichier du modèle du réseau de neurones", parent,
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Annuler", GTK_RESPONSE_CANCEL,
        "_Ouvrir", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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

    GtkWidget *progress_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(progress_window),
                         "Entraînement en cours...");
    gtk_window_set_default_size(GTK_WINDOW(progress_window), 300, 100);
    gtk_window_set_position(GTK_WINDOW(progress_window), GTK_WIN_POS_CENTER);

    gtk_window_set_modal(GTK_WINDOW(progress_window), TRUE);
    gtk_window_set_deletable(GTK_WINDOW(progress_window), FALSE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(progress_window), 20);
    gtk_container_add(GTK_CONTAINER(progress_window), vbox);

    GtkWidget *label = gtk_label_new("Entraînement du réseau de neurones...");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    GtkWidget *progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, TRUE, TRUE, 0);

    gtk_widget_show_all(progress_window);

    while (gtk_events_pending())
        gtk_main_iteration();

    train(neural, data.inputs, data.targets, data.samples, 0.01, 1000,
          update_progress_ui, progress_bar);

    gtk_widget_destroy(progress_window);
}

/*
 * get_neural_train_path:
 * Show folder chooser and start training on selected dataset.
 */
void get_neural_train_path(GtkWidget *widget)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choisir le modèle du réseau de neurones à charger", parent,
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Annuler", GTK_RESPONSE_CANCEL,
        "_Ouvrir", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
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
        "Choisir le fichier où enregistrer le modèle du réseau de neurones",
        parent, GTK_FILE_CHOOSER_ACTION_SAVE, "_Annuler", GTK_RESPONSE_CANCEL,
        "_Enregistrer", GTK_RESPONSE_ACCEPT, NULL);

    if (neural == NULL)
    {
        printf(COLOR_RED "[ERREUR] " COLOR_RESET
                         "Le modèle du réseau de neurones est vide!\n");
        return;
    }

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename)
        {
            save_network(neural, filename);
            printf(COLOR_GREEN "[SUCCESS] " COLOR_RESET
                               "Neural network saved to: %s\n",
                   filename);
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
    FcFini();

    return status;
}
