// Include gtk
#include <gtk/gtk.h>
#include <libgen.h>  // pour dirname
#include <limits.h>  // PATH_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // readlink

static void on_activate (GtkApplication *app) 
{
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) 
  {
      g_print("Pwd: %s\n", cwd);
  }

  // Create a new window
  GtkWidget *window = gtk_application_window_new (app);
  gtk_window_set_title(GTK_WINDOW(window), "Image");
  gtk_window_set_default_size(GTK_WINDOW(window), 2000, 1000);

  // Créer une box verticale (empile bouton + image)
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  // Obtenir le chemin absolu du binaire
  char exe_path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
  if (len == -1) 
  {
        g_print("ERROR: Failed to read binary path\n");
        return;
  }
  exe_path[len] = '\0';

  // Obtenir le dossier du binaire
  char *dir = dirname(exe_path);

  // Construire le chemin complet vers l'image
  char image_path[PATH_MAX];
  snprintf(image_path, sizeof(image_path), "%s/%s", dir, "level_1_image_1.png");
  g_print("Image path : %s\n", image_path);


  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
  if (!pixbuf)
  {
  	g_print("ERROR: Failed to load image\n");
	return;
  }
// Redimensionner à 1000x700
int win_width = 1000;  // largeur max souhaitée
int win_height = 700;  // hauteur max souhaitée

int img_width = gdk_pixbuf_get_width(pixbuf);
int img_height = gdk_pixbuf_get_height(pixbuf);

double img_ratio = (double)img_width / img_height;
double win_ratio = (double)win_width / win_height;

int new_width, new_height;
if (img_ratio > win_ratio) {
    new_width = win_width;
    new_height = (int)(win_width / img_ratio);
} else {
    new_height = win_height;
    new_width = (int)(win_height * img_ratio);
}

GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
GtkWidget *image = gtk_image_new_from_pixbuf(scaled);
  
  // Load image from file
  if (!gtk_image_get_paintable(GTK_IMAGE(image))) 
  {
      g_print("ERROR: Image not loaded. Check image path\n");
  }
  g_object_unref(pixbuf);
  g_object_unref(scaled);

// Étendre l'image
    gtk_widget_set_hexpand(image, TRUE);
    gtk_widget_set_vexpand(image, TRUE);

    // AspectFrame pour garder le ratio et remplir l'espace
    GtkWidget *frame = gtk_aspect_frame_new(0.5f, 0.5f, (float)new_width / new_height, TRUE);
    gtk_aspect_frame_set_child(GTK_ASPECT_FRAME(frame), image);
    gtk_widget_set_hexpand(frame, TRUE);
    gtk_widget_set_vexpand(frame, TRUE);

  GtkWidget *scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                               GTK_POLICY_AUTOMATIC,
                               GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), frame);
  gtk_box_append(GTK_BOX(box), scrolled);


  // Create a new button
  GtkWidget *button = gtk_button_new_with_label ("Hello, World!");
  // Ajouter bouton dans la box
  gtk_box_append(GTK_BOX(box), button);


  // Ajouter la box à la fenêtre
  gtk_window_set_child(GTK_WINDOW(window), box);


  // When the button is clicked, close the window passed as an argument
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_close), window);
  gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication *app = gtk_application_new ("com.example.GtkApplication",
                                             G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  return g_application_run (G_APPLICATION (app), argc, argv);
}

