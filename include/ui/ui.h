#pragma once

#include "../grid_cutting/grid_cutting.h"
#include "css.h"
#include "draw_solved_words.h"
#include "image_processing_buttons.h"
#include "neural_network.h"
#include "neural_network_buttons.h"
#include "popups.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

/**
 * AppData:
 * Small container for application state passed to callbacks.
 *
 * Fields:
 *  - filename      : the name of the of the image.
 *  - image         : GTK image widget showing the current pixbuf.
 *  - original      : original loaded GdkPixbuf (keep to allow reset).
 *  - transformed   : working copy that receives in-place treatments.
 *  - current       : pixbuf currently shown (may be rotated view of
 * transformed).
 *  - rotation_angle: accumulated rotation angle in degrees.
 *  - save_index    : counter used to generate unique filenames when saving.
 *  - pipelineResult: result of the pipeline to draw rectangles when solving the
 * grid.
 */
typedef struct
{
    char *filename;
    GtkWidget *image;
    GdkPixbuf *original;
    GdkPixbuf *transformed;
    GdkPixbuf *current;
    double rotation_angle;
    int save_index;
    PipelineResult *pipelineResult;
} AppData;

extern GtkWidget *solve_button;
extern NeuralNetwork *neural;
extern int is_processed;

gboolean update_image(gpointer user_data);
void apply_transformations(AppData *data);

void free_app_data(GtkWidget *widget __attribute__((unused)),
                   gpointer user_data);

gpointer load_pipeline_thread(gpointer user_data);
gpointer load_neural_thread(gpointer user_data);

void on_activate(GtkApplication *app, gpointer user_data);

int on_command_line(GApplication *app, GApplicationCommandLine *cmdline,
                    gpointer user_data __attribute__((unused)));

int main(int argc, char *argv[]);
