#include "../../include/ui/popups.h"

#include <gtk/gtk.h>

/*
 * pop_up_already_processed:
 * Show a small popup indicating the image was already processed.
 */
void pop_up_already_processed()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);
    GtkWidget *label = gtk_label_new("L'image a déja été traitée");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_widget_show_all(window);
}

/*
 * pop_up_solver_failure:
 * Show a small popup indicating the solver couldn't find any solved words in
 * the image given due to an error.
 */
void pop_up_solver_failure()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_POPUP);
    GtkWidget *label =
        gtk_label_new("Aucun mot n'a été détecté dans la grille !");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_widget_show_all(window);
}
