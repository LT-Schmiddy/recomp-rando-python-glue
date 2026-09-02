#include "main.h"
#include "recompui.h"

static RecompuiContext context;
static RecompuiResource root;

void RandoGlue_InitErrorContainer() {
    RecompuiColor error_color;
    error_color.r = 255;
    error_color.g = 0;
    error_color.b = 0;
    error_color.a = 255;

    context = recompui_create_context();
    recompui_open_context(context);

    // Allow the game to still be played behind this message.
    recompui_set_context_captures_input(context, false);
    recompui_set_context_captures_mouse(context, false);

    root = recompui_context_root(context);

    // Set up the root element so it takes up the full screen.
    recompui_set_position(root, POSITION_ABSOLUTE);
    recompui_set_top(root, 0, UNIT_DP);
    recompui_set_right(root, 0, UNIT_DP);
    recompui_set_bottom(root, 0, UNIT_DP);
    recompui_set_left(root, 0, UNIT_DP);
    recompui_set_width_auto(root);
    recompui_set_height_auto(root);
    
    // Set up the flexbox properties of the root element.
    recompui_set_display(root, DISPLAY_FLEX);
    recompui_set_flex_direction(root, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(root, JUSTIFY_CONTENT_CENTER);
    recompui_set_align_items(root, ALIGN_ITEMS_CENTER);

    // Add error message element directly to the root.
    RecompuiResource error_text = recompui_create_label(context, root, "An error has occured, please contact the developers.", LABELSTYLE_LARGE);
    recompui_set_color(error_text, &error_color);

    recompui_close_context(context);
}

// Shows the error message permanently until the game is closed.
void RandoGlue_ShowErrorContainer() {
    static bool context_shown = false;
    if (!context_shown) {
        recompui_show_context(context);
        context_shown = true;
    }
}