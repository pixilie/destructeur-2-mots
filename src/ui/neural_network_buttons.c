#include "../../include/neural_network.h"
#include "../../include/dataset.h"

#include "../../include/ui/ui.h"

#include <gtk/gtk.h>

// Terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

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
 * update_progress_ui:
 * Update the progress bar for the training of the neural network.
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
 * train_neural:
 * Create (if needed) and train a neural network using a dataset.
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
