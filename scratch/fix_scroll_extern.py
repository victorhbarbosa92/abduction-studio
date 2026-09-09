with open("src/ui/KuroPlaylistUI.h", "r", encoding="utf-8") as f:
    kuro = f.read()

# 1. Add externs inside namespace KuroUI
target_ns = """namespace KuroUI {
    extern bool show_piano_roll;
    extern bool show_psy_arranger_modal;"""

replacement_ns = """namespace KuroUI {
    extern bool show_piano_roll;
    extern bool show_psy_arranger_modal;
    extern float g_playlist_scroll_x;
    extern float g_playlist_scroll_y;"""

assert target_ns in kuro, "target_ns not found"
kuro = kuro.replace(target_ns, replacement_ns)

# 2. In render(), remove local extern declarations and use the namespace ones
target_render = """            if (!scroll_initialized) {
                extern float g_playlist_scroll_x;
                extern float g_playlist_scroll_y;
                if (g_playlist_scroll_x > 0.0f) playlist_scroll_x = g_playlist_scroll_x;
                if (g_playlist_scroll_y > 0.0f) playlist_scroll_y = g_playlist_scroll_y;
                scroll_initialized = true;
            }"""

replacement_render = """            if (!scroll_initialized) {
                if (g_playlist_scroll_x > 0.0f) playlist_scroll_x = g_playlist_scroll_x;
                if (g_playlist_scroll_y > 0.0f) playlist_scroll_y = g_playlist_scroll_y;
                scroll_initialized = true;
            }"""

assert target_render in kuro, "target_render not found"
kuro = kuro.replace(target_render, replacement_render)

with open("src/ui/KuroPlaylistUI.h", "w", encoding="utf-8") as f:
    f.write(kuro)

print("KuroPlaylistUI.h namespace KuroUI scroll externs fixed!")
