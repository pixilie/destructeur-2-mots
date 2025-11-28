#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

// Draws a red line from x to y in the UI image
void draw_line(GdkPixbuf *pixbuf, int x, int y)
{
    (void)pixbuf;
    (void)x;
    (void)y;
}

// Draws a red rectangle on the UI image, marking a word as resolved
void draw_rectangle(GdkPixbuf *pixbuf, int x1, int y1, int x2, int y2)
{
    draw_line(pixbuf, x1, x2);
    draw_line(pixbuf, x1, y1);
    draw_line(pixbuf, y1, y2);
    draw_line(pixbuf, x2, y2);
}

void find_solved_word()
{
    
}

void solve_grid()
{
    find_solved_word();
}


