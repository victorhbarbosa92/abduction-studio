import re

def update_playlist_ui():
    path = r"C:\Users\USUÁRIO\.gemini\antigravity-ide\scratch\abduction_studio_v2\src\ui\KuroPlaylistUI.h"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    # Check state variables in KuroPlaylistUI
    if "snap_to_zero_crossing" not in content:
        # insert state variables near playlist_zoom_x
        content = content.replace(
            "float playlist_scroll_x = 0.0f;\n        float playlist_scroll_y = 0.0f;",
            "float playlist_scroll_x = 0.0f;\n        float playlist_scroll_y = 0.0f;\n        bool snap_to_zero_crossing = false;\n        bool follow_playhead = true;"
        )

    print("State variables checked/inserted.")

if __name__ == "__main__":
    update_playlist_ui()
