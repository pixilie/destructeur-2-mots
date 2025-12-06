#pragma once

#include "line_detection.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

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
typedef struct
{
    GtkWidget *image;
    GdkPixbuf *original;
    GdkPixbuf *transformed;
    GdkPixbuf *current;
    double rotation_angle;
    int save_index;
    PipelineResult pipelineResult;
} AppData;

void draw_pixel(GdkPixbuf *pixbuf, int x, int y, int thickness);
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness);
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int x3,
                    int y3, int x4, int y4, int thickness);

int **get_all_words_coordinates(int rows, int cols, char tab[rows][cols],
                                int words_count, char **words);

int* get_word_image_coordinates(int grid_coos[4], int rows, int cols, int x1,
                                  int y1, int x2, int y2);

void solve_grid();
