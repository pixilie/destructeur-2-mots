#pragma once

#include "../grid_cutting/grid_cutting.h"
#include "../neural_network.h"

#include <gtk/gtk.h>

void automatic_treatement(GtkButton *button, gpointer user_data);
PipelineResult *load_pipeline(char *filename, NeuralNetwork *nn);
void solver(GtkButton *button, gpointer user_data);
void change_image(const char *filename, gpointer user_data);
void get_path_image(GtkWidget *widget, gpointer user_data);
void on_reset_clicked(GtkButton *button, gpointer user_data);
void on_save_clicked(GtkButton *button, gpointer user_data);
