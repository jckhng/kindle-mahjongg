#include "mahjongg_engine.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <cairo.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define APP_TITLE "Exact Mahjong Solitaire"
#define KINDLE_WINDOW_TITLE "L:A_N:application_ID:exactmahjongsolitaire_PC:N_O:URL"
#define KINDLE_WINDOW_TITLE_TOPBAR "L:A_N:application_PC:T_ID:exactmahjongsolitaire_O:URL"
#define LOG_PATH "/mnt/us/exact-mahjong-solitaire.log"
#define KINDLE_APP_WIDTH 1072
#define KINDLE_APP_HEIGHT 1448

typedef struct {
    GtkWidget *drawing;
    GtkWidget *status;
    GtkWidget *theme_combo;
    GdkPixbuf *tile_sheet;
    GdkPixbuf *tile_cache[2][43];
    int cache_w;
    int cache_h;
    char theme[32];
    MahjonggGame game;
    int selected;
    int hint_a;
    int hint_b;
    double scale;
    double origin_x;
    double origin_y;
    double tile_w;
    double tile_h;
} App;

static const char *kindle_window_title(void)
{
    const char *value = g_getenv("KINDLE_SHOW_TOPBAR");
    return (value != NULL && value[0] != '\0' && strcmp(value, "0") != 0) ? KINDLE_WINDOW_TITLE_TOPBAR
                                                                          : KINDLE_WINDOW_TITLE;
}

static void app_apply_high_contrast(GtkWidget *widget)
{
    GdkColor black = {0, 0x0000, 0x0000, 0x0000};
    GdkColor white = {0, 0xffff, 0xffff, 0xffff};

    gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &black);
    gtk_widget_modify_fg(widget, GTK_STATE_ACTIVE, &black);
    gtk_widget_modify_fg(widget, GTK_STATE_SELECTED, &white);
    gtk_widget_modify_text(widget, GTK_STATE_NORMAL, &black);
    gtk_widget_modify_text(widget, GTK_STATE_SELECTED, &white);
    gtk_widget_modify_base(widget, GTK_STATE_NORMAL, &white);
    gtk_widget_modify_base(widget, GTK_STATE_SELECTED, &black);
    gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &white);
    gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &black);
}

static void app_install_kindle_style(void)
{
    gtk_rc_parse_string(
        "style \"kindle_high_contrast\" {\n"
        "  fg[NORMAL] = \"#000000\"\n"
        "  fg[ACTIVE] = \"#000000\"\n"
        "  fg[PRELIGHT] = \"#ffffff\"\n"
        "  fg[SELECTED] = \"#ffffff\"\n"
        "  text[NORMAL] = \"#000000\"\n"
        "  text[ACTIVE] = \"#000000\"\n"
        "  text[SELECTED] = \"#ffffff\"\n"
        "  base[NORMAL] = \"#ffffff\"\n"
        "  base[ACTIVE] = \"#ffffff\"\n"
        "  base[SELECTED] = \"#000000\"\n"
        "  bg[NORMAL] = \"#ffffff\"\n"
        "  bg[ACTIVE] = \"#ffffff\"\n"
        "  bg[PRELIGHT] = \"#000000\"\n"
        "  bg[SELECTED] = \"#000000\"\n"
        "}\n"
        "gtk-button-images = 0\n"
        "gtk-menu-images = 0\n"
        "class \"GtkComboBox\" style \"kindle_high_contrast\"\n"
        "class \"GtkCellView\" style \"kindle_high_contrast\"\n"
        "class \"GtkMenu\" style \"kindle_high_contrast\"\n"
        "class \"GtkMenuItem\" style \"kindle_high_contrast\"\n"
        "widget_class \"*GtkComboBox*\" style \"kindle_high_contrast\"\n"
        "widget_class \"*GtkMenu*\" style \"kindle_high_contrast\"\n"
    );
}

static void app_log(const char *message)
{
    FILE *f = fopen(LOG_PATH, "a");
    if (!f)
        return;
    fprintf(f, "[app] %s\n", message);
    fclose(f);
}

static GdkPixbuf *load_tile_sheet(const char *theme)
{
    char kindle_path[256];
    char local_path[256];
    const char *paths[] = {
        kindle_path,
        local_path,
        NULL
    };
    int i;

    if (strcmp(theme, "smooth") == 0) {
        g_snprintf(kindle_path, sizeof(kindle_path), "/mnt/us/extensions/exact-mahjong-solitaire/assets/smooth.png");
        g_snprintf(local_path, sizeof(local_path), "assets/smooth.png");
    } else {
        g_snprintf(kindle_path, sizeof(kindle_path), "/mnt/us/extensions/exact-mahjong-solitaire/assets/postmodern.png");
        g_snprintf(local_path, sizeof(local_path), "assets/postmodern.png");
    }

    for (i = 0; paths[i] != NULL; i++) {
        GError *error = NULL;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(paths[i], &error);
        if (pixbuf != NULL)
            return pixbuf;
        if (error)
            g_error_free(error);
    }

    app_log("tile sheet load failed; using cairo fallback");
    return NULL;
}

static void clear_tile_cache(App *app)
{
    int i;
    int row;

    for (row = 0; row < 2; row++) {
        for (i = 0; i < 43; i++) {
            if (app->tile_cache[row][i]) {
                g_object_unref(app->tile_cache[row][i]);
                app->tile_cache[row][i] = NULL;
            }
        }
    }
    app->cache_w = 0;
    app->cache_h = 0;
}

static void reload_tile_sheet(App *app)
{
    if (app->tile_sheet) {
        g_object_unref(app->tile_sheet);
        app->tile_sheet = NULL;
    }
    clear_tile_cache(app);
    app->tile_sheet = load_tile_sheet(app->theme);
}

static void ensure_tile_cache(App *app, int target_w, int target_h)
{
    int sheet_w;
    int sheet_h;
    int src_w;
    int src_h;
    int src_y;
    int i;

    if (app->tile_sheet == NULL)
        return;
    if (target_w <= 0 || target_h <= 0)
        return;
    if (app->cache_w == target_w && app->cache_h == target_h && app->tile_cache[0][0] != NULL)
        return;

    clear_tile_cache(app);

    sheet_w = gdk_pixbuf_get_width(app->tile_sheet);
    sheet_h = gdk_pixbuf_get_height(app->tile_sheet);
    src_w = sheet_w / 43;
    src_h = sheet_h / 2;

    for (src_y = 0; src_y < 2; src_y++) {
        for (i = 0; i < 43; i++) {
            GdkPixbuf *sub = gdk_pixbuf_new_subpixbuf(app->tile_sheet, i * src_w, src_y * src_h, src_w, src_h);
            app->tile_cache[src_y][i] = gdk_pixbuf_scale_simple(sub, target_w, target_h, GDK_INTERP_BILINEAR);
            g_object_unref(sub);
        }
    }

    app->cache_w = target_w;
    app->cache_h = target_h;
}

static void draw_tile_bevel(cairo_t *cr, double x, double y, double w, double h, double depth)
{
    cairo_save(cr);

    cairo_rectangle(cr, x, y, w, h);
    cairo_clip(cr);

    cairo_set_source_rgba(cr, 0.20, 0.20, 0.18, 0.55);
    cairo_rectangle(cr, x, y, depth, h);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, 0.10, 0.10, 0.09, 0.50);
    cairo_rectangle(cr, x, y + h - depth, w, depth);
    cairo_fill(cr);

    cairo_restore(cr);
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    (void)widget;
    (void)data;

    if (event->keyval == GDK_Escape || event->keyval == GDK_q) {
        gtk_main_quit();
        return TRUE;
    }

    return FALSE;
}

static void app_update_status(App *app, const char *message)
{
    char text[160];
    int left = mahjongg_tiles_remaining(&app->game);

    g_snprintf(text, sizeof(text), "%s   Tiles: %d   Removed: %d", message, left, app->game.history_count * 2);
    gtk_label_set_text(GTK_LABEL(app->status), text);
}

static void app_new_game(App *app)
{
    mahjongg_game_init(&app->game, (unsigned int)time(NULL));
    app->selected = -1;
    app->hint_a = -1;
    app->hint_b = -1;
    app_update_status(app, "New game");
    gtk_widget_queue_draw(app->drawing);
}

static void tile_rect(App *app, const MahjonggTile *tile, double *x, double *y, double *w, double *h)
{
    double base_w = app->scale;
    double base_h = app->scale * 1.375;
    double offset_x = base_w / 7.0;
    double offset_y = base_h / 10.0;

    *x = app->origin_x + tile->x * base_w / 2.0 + tile->layer * offset_x;
    *y = app->origin_y + tile->y * base_h / 2.0 - tile->layer * offset_y;
    *w = base_w + offset_x;
    *h = base_h + offset_y;
}

static int tile_at(App *app, double px, double py)
{
    int i;
    int best = -1;

    for (i = 0; i < app->game.tile_count; i++) {
        MahjonggTile *tile = &app->game.tiles[i];
        double x, y, w, h;

        if (tile->removed)
            continue;

        tile_rect(app, tile, &x, &y, &w, &h);
        if (px >= x && px <= x + w && py >= y && py <= y + h) {
            if (best < 0 || tile->layer >= app->game.tiles[best].layer)
                best = i;
        }
    }

    return best;
}

static gboolean on_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    App *app = data;
    cairo_t *cr = gdk_cairo_create(widget->window);
    GtkAllocation alloc;
    int i;
    (void)event;

    gtk_widget_get_allocation(widget, &alloc);
    app->scale = fmin((alloc.width - 32.0) / 16.0, (alloc.height - 32.0) / (9.0 * 1.375));
    if (app->scale < 12.0)
        app->scale = 12.0;
    app->tile_w = app->scale + app->scale / 7.0;
    app->tile_h = app->scale * 1.375 + (app->scale * 1.375) / 10.0;
    app->origin_x = (alloc.width - 15.0 * app->scale - app->tile_w) / 2.0 + app->scale / 2.0;
    app->origin_y = (alloc.height - 8.0 * app->scale * 1.375 - app->tile_h) / 2.0 +
                    app->scale * 1.375 / 2.0;

    cairo_set_source_rgb(cr, 0.78, 0.78, 0.74);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    for (i = app->game.tile_count - 1; i >= 0; i--) {
        MahjonggTile *tile = &app->game.tiles[i];
        double x, y, w, h;

        if (tile->removed)
            continue;

        tile_rect(app, tile, &x, &y, &w, &h);

        if (app->tile_sheet != NULL) {
            int image = tile->kind;
            int target_w = MAX(1, (int)w);
            int target_h = MAX(1, (int)h);
            int skin = (i == app->selected || i == app->hint_a || i == app->hint_b) ? 1 : 0;

            ensure_tile_cache(app, target_w, target_h);

            cairo_save(cr);
            cairo_rectangle(cr, x, y, w, h);
            cairo_clip(cr);
            if (app->tile_cache[skin][image] != NULL) {
                gdk_cairo_set_source_pixbuf(cr, app->tile_cache[skin][image], x, y);
                cairo_paint(cr);
            }
            cairo_restore(cr);
        } else {
            int free_tile = mahjongg_is_free(&app->game, i);

            cairo_set_source_rgb(cr, 0.24, 0.24, 0.22);
            cairo_rectangle(cr, x + 3, y + 3, w, h);
            cairo_fill(cr);

            if (i == app->selected)
                cairo_set_source_rgb(cr, 0.98, 0.94, 0.65);
            else if (i == app->hint_a || i == app->hint_b)
                cairo_set_source_rgb(cr, 0.86, 0.90, 0.70);
            else if (free_tile)
                cairo_set_source_rgb(cr, 0.96, 0.96, 0.90);
            else
                cairo_set_source_rgb(cr, 0.68, 0.68, 0.64);

            cairo_rectangle(cr, x, y, w, h);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
            cairo_set_line_width(cr, 1.5);
            cairo_stroke(cr);

            cairo_set_font_size(cr, app->scale * 0.48);
            cairo_move_to(cr, x + app->scale * 0.22, y + app->scale * 1.05);
            cairo_show_text(cr, mahjongg_kind_label(tile->kind));

            draw_tile_bevel(cr, x, y, w, h, app->scale / 9.0);

            if (free_tile) {
                cairo_set_line_width(cr, 2.0);
                cairo_rectangle(cr, x + 3, y + 3, w - 6, h - 6);
                cairo_stroke(cr);
            }
        }
    }

    cairo_destroy(cr);
    return FALSE;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    App *app = data;
    int idx;
    (void)widget;

    if (event->button != 1)
        return FALSE;

    idx = tile_at(app, event->x, event->y);
    app->hint_a = -1;
    app->hint_b = -1;

    if (idx < 0 || !mahjongg_is_free(&app->game, idx)) {
        app->selected = -1;
        app_update_status(app, "Choose a free tile");
    } else if (app->selected < 0) {
        app->selected = idx;
        app_update_status(app, "Select matching tile");
    } else if (idx == app->selected) {
        app->selected = -1;
        app_update_status(app, "Selection cleared");
    } else if (mahjongg_apply_match(&app->game, app->selected, idx)) {
        app->selected = -1;
        if (mahjongg_tiles_remaining(&app->game) == 0)
            app_update_status(app, "Solved");
        else
            app_update_status(app, "Matched");
    } else {
        app->selected = idx;
        app_update_status(app, "Not a match");
    }

    gtk_widget_queue_draw(app->drawing);
    return TRUE;
}

static void on_new_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    app_new_game(data);
}

static void on_restart_clicked(GtkButton *button, gpointer data)
{
    App *app = data;
    (void)button;

    mahjongg_game_restart(&app->game);
    app->selected = -1;
    app->hint_a = -1;
    app->hint_b = -1;
    app_update_status(app, "Restarted");
    gtk_widget_queue_draw(app->drawing);
}

static void on_hint_clicked(GtkButton *button, gpointer data)
{
    App *app = data;
    (void)button;

    app->selected = -1;
    if (mahjongg_find_hint(&app->game, &app->hint_a, &app->hint_b))
        app_update_status(app, "Hint highlighted");
    else
        app_update_status(app, "No moves left");

    gtk_widget_queue_draw(app->drawing);
}

static void on_undo_clicked(GtkButton *button, gpointer data)
{
    App *app = data;
    (void)button;

    app->selected = -1;
    app->hint_a = -1;
    app->hint_b = -1;
    if (mahjongg_undo(&app->game))
        app_update_status(app, "Undone");
    else
        app_update_status(app, "Nothing to undo");

    gtk_widget_queue_draw(app->drawing);
}

static void on_quit_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;
    gtk_main_quit();
}

static void on_theme_changed(GtkComboBox *combo, gpointer data)
{
    App *app = data;
    int active = gtk_combo_box_get_active(combo);

    g_strlcpy(app->theme, active == 1 ? "smooth" : "postmodern", sizeof(app->theme));
    reload_tile_sheet(app);
    app->selected = -1;
    app->hint_a = -1;
    app->hint_b = -1;
    app_update_status(app, active == 1 ? "Theme: Smooth" : "Theme: Postmodern");
    gtk_widget_queue_draw(app->drawing);
}

int main(int argc, char **argv)
{
    App app;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *bar;
    GtkWidget *button;
    GtkWidget *title;

    app_log("startup");
    gtk_init(&argc, &argv);
    app_install_kindle_style();
    memset(&app, 0, sizeof(app));
    g_strlcpy(app.theme, "postmodern", sizeof(app.theme));
    app.tile_sheet = load_tile_sheet(app.theme);
    app.selected = -1;
    app.hint_a = -1;
    app.hint_b = -1;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), kindle_window_title());
    gtk_window_set_default_size(GTK_WINDOW(window), KINDLE_APP_WIDTH, KINDLE_APP_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_move(GTK_WINDOW(window), 0, 0);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    vbox = gtk_vbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    title = gtk_label_new(APP_TITLE);
    gtk_misc_set_alignment(GTK_MISC(title), 0.5f, 0.5f);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    app.status = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(app.status), 0.5f, 0.5f);
    gtk_box_pack_start(GTK_BOX(vbox), app.status, FALSE, FALSE, 0);

    bar = gtk_hbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vbox), bar, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("New");
    g_signal_connect(button, "clicked", G_CALLBACK(on_new_clicked), &app);
    gtk_box_pack_start(GTK_BOX(bar), button, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Restart");
    g_signal_connect(button, "clicked", G_CALLBACK(on_restart_clicked), &app);
    gtk_box_pack_start(GTK_BOX(bar), button, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Hint");
    g_signal_connect(button, "clicked", G_CALLBACK(on_hint_clicked), &app);
    gtk_box_pack_start(GTK_BOX(bar), button, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Undo");
    g_signal_connect(button, "clicked", G_CALLBACK(on_undo_clicked), &app);
    gtk_box_pack_start(GTK_BOX(bar), button, TRUE, TRUE, 0);

    app.theme_combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(app.theme_combo), "Postmodern");
    gtk_combo_box_append_text(GTK_COMBO_BOX(app.theme_combo), "Smooth");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app.theme_combo), 0);
    app_apply_high_contrast(app.theme_combo);
    g_signal_connect(app.theme_combo, "changed", G_CALLBACK(on_theme_changed), &app);
    gtk_box_pack_start(GTK_BOX(bar), app.theme_combo, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Quit");
    g_signal_connect(button, "clicked", G_CALLBACK(on_quit_clicked), &app);
    gtk_box_pack_start(GTK_BOX(bar), button, TRUE, TRUE, 0);

    app.drawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(app.drawing, 1030, 1280);
    gtk_widget_add_events(app.drawing, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(app.drawing, "expose-event", G_CALLBACK(on_draw), &app);
    g_signal_connect(app.drawing, "button-press-event", G_CALLBACK(on_button_press), &app);
    gtk_box_pack_start(GTK_BOX(vbox), app.drawing, TRUE, TRUE, 0);

    app_new_game(&app);
    gtk_widget_show_all(window);
    gtk_window_present(GTK_WINDOW(window));
    app_log("window shown");
    gtk_main();
    if (app.tile_sheet)
        g_object_unref(app.tile_sheet);
    clear_tile_cache(&app);
    app_log("shutdown");
    return 0;
}
