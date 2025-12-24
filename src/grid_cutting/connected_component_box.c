#include "../../include/grid_cutting/connected_component_box.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdlib.h>

typedef struct
{
    int x;
    int y;
} Point;

/**
 * find_white_pixels_around:
 * Recursive flood-fill helper that expands a connected component of white
 * pixels (considered foreground) and updates a bounding box.
 *
 * Parameters:
 *  - pixbuf     : binarized GdkPixbuf where foreground pixels have value 255.
 *  - x, y       : starting coordinates for the search.
 *  - is_visited : int array of size width*height marking visited pixels (0/1).
 *  - index_coo  : index into coo array for storing bounding box values.
 *  - coo        : array of integer arrays; coo[index_coo] holds
 * [xmin,ymin,xmax,ymax].
 */
void find_white_pixels_around(GdkPixbuf *pixbuf, int start_x, int start_y,
                              int *is_visited, int index_coo, int **coo)
{
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

    Point *queue = malloc(width * height * sizeof(Point));
    int front = 0;
    int back = 0;

    queue[back] = (Point){start_x, start_y};
    back++;

    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                            {0, 1},   {1, -1}, {1, 0},  {1, 1}};

    while (front < back)
    {
        Point point = queue[front];
        front++;

        int x = point.x;
        int y = point.y;

        guchar *p = pixels + y * rowstride + x * n_channels;

        if (p[0] < 200 || is_visited[y * width + x])
        {
            continue;
        }

        is_visited[y * width + x] = 1;

        if (x < coo[index_coo][0])
            coo[index_coo][0] = x;
        if (y < coo[index_coo][1])
            coo[index_coo][1] = y;
        if (x > coo[index_coo][2])
            coo[index_coo][2] = x;
        if (y > coo[index_coo][3])
            coo[index_coo][3] = y;

        for (int i = 0; i < 8; i++)
        {
            int nx = x + directions[i][0];
            int ny = y + directions[i][1];
            if (nx >= 0 && ny >= 0 && nx < width && ny < height)
            {
                queue[back] = (Point){nx, ny};
                back++;
            }
        }
    }

    free(queue);
}
