import os

# We will read src/ui/KuroPlaylistUI.h and replace renderModernToolbar and renderClipsForTrack
with open("src/ui/KuroPlaylistUI.h", "r", encoding="utf-8") as f:
    content = f.read()

# Let's verify markers
assert "void renderModernToolbar" in content, "renderModernToolbar not found"
assert "void renderClipsForTrack" in content, "renderClipsForTrack not found"

print("Read KuroPlaylistUI.h, size:", len(content))
