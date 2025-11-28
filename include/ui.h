#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness);
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2,
                    int thickness);
void find_solved_word();
void solve_grid();
