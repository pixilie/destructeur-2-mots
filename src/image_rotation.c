#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>

GdkPixbuf* rotate_image(GdkPixbuf *src, double angle_degrees)
{
	int width = gdk_pixbuf_get_width(src);
	int height = gdk_pixbuf_get_height(src);
	
	//Convert degrees to radians
	double angle = angle_degrees * M_PI / 180;
	
	//TODO
	return src;
}
