import subprocess
import os
import shutil

# -----------------------------
# Anchor paths
# -----------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

source_file = os.path.join(BASE_DIR, "main.c")

build_dir = os.path.join(BASE_DIR, "build", "gcc_single")
os.makedirs(build_dir, exist_ok=True)

output_file = os.path.join(build_dir, "hammer_ironclib.exe")

# -----------------------------
# Check GCC
# -----------------------------
def tool_exists(cmd):
    return shutil.which(cmd) is not None

if not tool_exists("gcc"):
    print("❌ gcc not found in PATH")
    raise SystemExit(1)

# -----------------------------
# Compile
# -----------------------------
print("🔧 GCC build (C11, -O2)")

subprocess.check_call([
    "gcc",
    "-std=c11",
    "-O2",
    source_file,
    "-o",
    output_file
])

print("✔ Build success")

# -----------------------------
# Run
# -----------------------------
print("\n🚀 Running...\n")

result = subprocess.run(output_file)
print(f"\n🏁 Exit code: {result.returncode}")