#include <gtk/gtk.h>

void draw_pixel(GdkPixbuf *pixbuf, int x, int y, int thickness, int color[3]);
void draw_line(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int thickness,
               int color[3]);
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2, int x3,
                    int y3, int x4, int y4, int thickness);
