import os
import sys

export_dir = r"scratch/tracks/export"
pioneer_dir = os.path.join(export_dir, "PIONEER", "rekordbox")
xml_file = os.path.join(pioneer_dir, "export.xml")

# Let's test creating sample files if not yet run or check
os.makedirs(pioneer_dir, exist_ok=True)
print(f"Checking export dir: {export_dir}")
