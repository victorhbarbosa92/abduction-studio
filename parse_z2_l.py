with open("z2_l_dump.txt", "r", encoding="utf-8") as f:
    text = f.read()

# Let's find where the arrays d, e, f, j, k, l, m, n, o are filled in z2/l
methods = text.split("Direct methods    -")[-1]
print("Methods in z2/l:", [line for line in methods.split("\n") if "name          :" in line])

# Let's search for fill-array-data or aput in z2/l
import re
fill_data = re.findall(r'fill-array-data.*', text)
print(f"Fill array data instructions: {len(fill_data)}")
