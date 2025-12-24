#pragma once

#include <gtk/gtk.h>

void load_neural(const char *filename);
void get_neural_load_path(GtkWidget *widget);
void update_progress_ui(int current_epoch, int total_epochs, void *user_data);
void train_neural(const char *filename);
void get_neural_train_path(GtkWidget *widget);
void save_neural(GtkWidget *widget);
