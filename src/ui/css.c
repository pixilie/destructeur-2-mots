#include "../../include/ui/css.h"

#include <gtk/gtk.h>

void load_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(
        provider,
        "window { background-color: white; }"
        "#app-title { font-size: 24px; font-weight: bold; margin: 10px; }"
        ".quit-btn { color: red; background: white; border: 2px solid red; "
        "border-radius: 5px; font-weight: bold; margin: 10px; }"
        ".quit-btn:hover { background: #ffe6e6; }"
        "#image-border { border: 4px solid #4CAF50; border-radius: 8px; "
        "margin: 10px; background-color: white; }"
        ".bold-label { font-weight: bold; font-size: 14px; margin-bottom: 5px; "
        "}"
        "button { background: white; border: 2px solid black; border-radius: "
        "8px; padding: 5px 10px; font-weight: bold; margin: 5px; }"
        "button:hover { background: #f0f0f0; }",
        -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}
