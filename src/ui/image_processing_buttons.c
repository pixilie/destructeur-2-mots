#include "../../include/image_processing/image_processing.h"
#include "../../include/ui/ui.h"

#include <gtk/gtk.h>

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

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
        pop_up_already_processed();
    }
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
        pop_up_solver_failure();
        return;
    }

    if (words->detected_words_count == 0)
    {
        printf(COLOR_RED "[ERREUR] " COLOR_RESET
                         "Aucun mot n'a été détecté dans la grille!\n");
        pop_up_solver_failure();
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
            printf(COLOR_YELLOW "[SOLVER] " COLOR_RESET "Rectangle drawn at (%i, %i) (%i, %i) (%i, %i) (%i, %i) "
                   "with thickness %i\n",
                   x1, y1, x2, y2, x3, y3, x4, y4, thickness);
        }
    }

    if (is_drawn == 0)
    {
        printf(COLOR_RED
               "[ERREUR] " COLOR_RESET
               "Aucun mot n'a été marqué comme résolu dans la grille\n");
        pop_up_solver_failure();
    }

    data->rotation_angle = 0;
    apply_transformations(data);
    data->rotation_angle = rotation_angle;
}

/*
 * change_image:
 * Load image from filename and update AppData buffers.
 */
void change_image(const char *filename, gpointer user_data)
{
    AppData *data = user_data;
    is_processed = 0;
    gtk_widget_set_sensitive(solve_button, FALSE);

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

    if (data->filename)
    {
        g_free(data->filename);
    }
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
    
    data->filename = g_strdup(filename);
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

    // Load pipeline in a separate thread to prevent UI from being blocked while
    // the pipeline is running
    g_thread_new("pipeline_thread", load_pipeline_thread, data);
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
