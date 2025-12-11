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

#include "../include/neural_network.h" 
#include "../include/dataset.h"

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

NeuralNetwork *neural;
int treated = 0;


/**
 * update_image:
 * Update the GTK image widget to display data->current and request redraw.
 *
 * Parameters:
   - data: pointer to application state (must have data->image and
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

    treated = 0; 
    //because we reset the img, it become untreated again

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
void on_save_clicked(GtkButton *button, gpointer user_data, GtkWidget *widget)
{
    (void)button;
    struct AppData *data = user_data;

    if (!data->current)
    {
        return;
    }

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
		    "Choisir où sauvegarder",
		    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
		    GTK_FILE_CHOOSER_ACTION_SAVE,
		    "_Annuler", GTK_RESPONSE_CANCEL,
		    "_Ouvrir", GTK_RESPONSE_ACCEPT,
		    NULL);
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
	char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	if(filename)
	{
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
    }

    gtk_widget_destroy(dialog);
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


void pop_up_treated()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);

    GtkWidget *text= gtk_label_new("Image already treated");
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_box_pack_start(GTK_BOX(box), text, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
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

    if(treated == 0)
    {
        if(!data->transformed)
		return;

        convert_to_grayscale(data->transformed);
        int treshold = convert_to_black_and_white(data->transformed);
        printf("threshold = %i\n", treshold);
	median_filter_3x3(data->transformed);
        data->transformed = rotate_image_automatic(data->transformed);

        apply_transformations(data);

	treated = 1;
    }
    else
    {
	printf("Image already treated\n");
	pop_up_treated();
    }
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


void change_image(const char *filename, gpointer user_data)
{
    struct AppData *data = (struct AppData *)user_data;

    treated = 0;

    // Basics verification
    if (!data) {
        g_printerr("Erreur : AppData invalide\n");
        return;
    }
    if (!data->image) {
        g_printerr("Erreur : data->image n'est pas initialisé\n");
        return;
    }
    if (!filename || filename[0] == '\0') {
        g_printerr("Erreur : chemin d'image invalide\n");
        return;
    }

    g_print("DEBUG: data=%p, data->image=%p, filename=%s\n",
            data, data->image, filename);

    // load pixbuf from file
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (!pixbuf) {
        g_printerr("Erreur chargement image '%s': %s\n",
                   filename, error ? error->message : "unknown");
        if (error) g_error_free(error);
        return;
    }

    g_print("DEBUG: pixbuf=%p, width=%d, height=%d\n", pixbuf,
            gdk_pixbuf_get_width(pixbuf),
            gdk_pixbuf_get_height(pixbuf));

    // Change to the new image 
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->image), pixbuf);

    // Free older pixbuf if needed
    if (data->original) {
        g_object_unref(data->original);
        data->original = NULL;
    }
    if (data->current) {
        g_object_unref(data->current);
        data->current = NULL;
    }
    if (data->transformed) {
        g_object_unref(data->transformed);
        data->transformed = NULL;
    }

    data->original = pixbuf;

    data->current = gdk_pixbuf_copy(pixbuf);
    if (!data->current) {
        g_printerr("DEBUG: impossible de copier pixbuf pour current\n");
    } else {
        g_print("DEBUG: current pixbuf=%p\n", data->current);
    }

    data->transformed = gdk_pixbuf_copy(pixbuf);
    if (!data->transformed) {
        g_printerr("DEBUG: impossible de copier pixbuf pour transformed\n");
    } else {
        g_print("DEBUG: transformed pixbuf=%p\n", data->transformed);
    }

    // Reset other parameters
    data->rotation_angle = 0.0;
    data->save_index = 1;

    g_print("DEBUG: change_image terminé\n");
}


void get_path_image(GtkWidget *widget, gpointer user_data)
{
    struct AppData *data = (struct AppData *)user_data;
    if (!data) {
        g_printerr("Erreur : AppData invalide dans get_path_image\n");
        return;
    }

    // Creation of dialog 
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choisir une image",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)), // parent window
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Annuler", GTK_RESPONSE_CANCEL,
        "_Ouvrir", GTK_RESPONSE_ACCEPT,
        NULL
    );

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) 
	{
            printf("Image choisie : %s\n", filename);

            // Call to the function to change the image
            change_image(filename, data);

            g_free(filename);
        }
       	else 
	{
            g_printerr("Erreur : aucun fichier sélectionné\n");
        }
    }

    gtk_widget_destroy(dialog);
}


void load_neural(const char *filename)
{
    printf("neural to load is here : %s\n", filename);
    neural = load_network(filename);
}

void get_neural_load_path(GtkWidget *widget)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
		    "Choisir le fichier :",
		    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
		    GTK_FILE_CHOOSER_ACTION_OPEN,
		    "_Annuler", GTK_RESPONSE_CANCEL,
		    "_Ouvrir", GTK_RESPONSE_ACCEPT,
		    NULL);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
	char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	if(filename)
	{
	    printf("Neural choisi : %s\n", filename);

	    load_neural(filename);
	    g_free(filename);
	}
	else
	{
	    g_printerr("Erreur : aucun fichier selectionné\n");
	}
    }
    gtk_widget_destroy(dialog);
}


void train_neural(const char *filename)
{
    char *path = malloc(256);
    if(path == NULL)
	    return;
    path = filename;

    if(neural == NULL)
	    neural = create_network(784, 128, 26);

    Dataset data = load_dataset(filename);
    train(neural, data.inputs, data.targets, data.samples, 0.01, 1000);
}


void get_neural_train_path(GtkWidget *widget)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
		    "Choisir le Dataset",
		    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
		    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		    "_Annuler", GTK_RESPONSE_CANCEL,
		    "_Ouvrir", GTK_RESPONSE_ACCEPT,
		    NULL);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
	char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	if(filename)
	{
	    printf("Neural choisi : %s\n", filename);

	    train_neural(filename);
	    g_free(filename);
	}
	else
	{
	    g_printerr("Erreur : aucun fichier selectionné\n");
	}
    }

    gtk_widget_destroy(dialog);
}


void save_neural(GtkWidget *widget)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
		    "Choisir où sauvegarder",
		    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
		    GTK_FILE_CHOOSER_ACTION_SAVE,
		    "_Annuler", GTK_RESPONSE_CANCEL,
		    "_Ouvrir", GTK_RESPONSE_ACCEPT,
		    NULL);

    if(neural == NULL)
    {
        printf("Error : cannot save empty NeuralNetwork\n");
        return;
    }
    else
    {
        if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        {
	    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	    if(filename)
	    {
	        save_network(neural, filename);
	        printf("Save neural at : %s\n", filename);
	    }
        }
        gtk_widget_destroy(dialog);
    }
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

    char **filename = (char**)user_data;
    printf("Fichier à charger : %s\n", *filename);

    const char *image_path = get_image_path(*filename);
    if (!image_path)
    {
        g_printerr("Could not resolve image path for '%s'\n", *filename);
        return;
    }
    printf("Loading image at: %s\n", image_path);


    //button to choose file
    file_button = gtk_button_new_with_label("Charger l'image");

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
    //Size of window, check if size-auto possible
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1000);
    

    // Create a vertical box
    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vertical_box);


    //Create vertical box for header
    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vertical_box),top_bar, FALSE, FALSE, 0);

    //label
    GtkWidget *title = gtk_label_new("Destructeur 2 mots");
    gtk_box_pack_start(GTK_BOX(top_bar), title, FALSE, FALSE, 0);
    //space between title and quit
    GtkWidget *space = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), space, TRUE, TRUE, 0);
    GtkWidget *space2 = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), space2, TRUE, TRUE, 0);
    GtkWidget *space3 = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), space3, TRUE, TRUE, 0);
    //Quit button
    close_button = gtk_button_new_with_label("Quitter");
    g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_close), window);
    gtk_box_pack_start(GTK_BOX(top_bar), close_button, TRUE, TRUE, 15);


    GdkPixbuf *scaled =
        gdk_pixbuf_scale_simple(pixbuf, 1000, 700, GDK_INTERP_BILINEAR);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    image = gtk_image_new_from_pixbuf(scaled);
    if (!gtk_image_get_pixbuf(GTK_IMAGE(image)))
    {
        g_print("ERROR: Image not loaded. Check image path\n");
        return;
    }
    gtk_container_add(GTK_CONTAINER(scrolled), image);


    //Create box for neural training
    center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical_box), center, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(center), scrolled, TRUE, TRUE, 0);
    
    //right button for neural
    right_button = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(center), right_button, FALSE, FALSE, 0);


    horizontal_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vertical_box), horizontal_box, FALSE, FALSE, 5);
   //Create new buttons for image action

   treatement_button = gtk_button_new_with_label("Traiter");
   reset_button = gtk_button_new_with_label("Réinitialiser");
   save_button = gtk_button_new_with_label("Sauvegarder");
   solver_button = gtk_button_new_with_label("Resoudre");

   description = gtk_label_new("Image :");

   gtk_box_pack_start(GTK_BOX(horizontal_box), treatement_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), solver_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), reset_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(horizontal_box), save_button, TRUE, TRUE, 5);


   // Initialize AppData
    struct AppData *data = g_new(struct AppData, 1);
    data->image = image;
    data->original = pixbuf;
    data->current = gdk_pixbuf_copy(pixbuf);
    data->transformed = gdk_pixbuf_copy(pixbuf);
    data->rotation_angle = 0.0;
    data->save_index = 1;
   
    
    //label
   title_neural = gtk_label_new("Réseau de neurone :");
   gtk_box_pack_start(GTK_BOX(right_button), title_neural, FALSE, FALSE, 0);
   //Create new button for neural training
   training_button = gtk_button_new_with_label("Entraîner et créer un réseau");
   g_signal_connect(training_button, "clicked", G_CALLBACK(get_neural_train_path), NULL); //check NULL

   load_button = gtk_button_new_with_label("Charger un modèle déjà existant");
   g_signal_connect(load_button, "clicked", G_CALLBACK(get_neural_load_path), NULL);

   save_neural_button = gtk_button_new_with_label("Sauvegarder l'entraînement");
   g_signal_connect(save_neural_button, "clicked", G_CALLBACK(save_neural), NULL);


   gtk_box_pack_start(GTK_BOX(right_button), file_button, FALSE, TRUE, 5);
   g_signal_connect(file_button, "clicked", G_CALLBACK(get_path_image), data); //here


   gtk_box_pack_start(GTK_BOX(right_button), training_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(right_button), load_button, TRUE, TRUE, 5);
   gtk_box_pack_start(GTK_BOX(right_button), save_neural_button, TRUE, TRUE, 5);


    g_signal_connect(treatement_button, "clicked", G_CALLBACK(automatic_treatement), data);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_clicked), data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), data);
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
        //filename = argv[1];
	filename = g_strdup(argv[1]);
    }
    else
    {
        //filename = "level_1_image_1.png";
	filename = g_strdup("level_1_image_1.png");
    }

    // Create a new application
    app = gtk_application_new("com.example.GtkApplication",
                              G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &filename);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}


//int main() { return EXIT_SUCCESS; }
