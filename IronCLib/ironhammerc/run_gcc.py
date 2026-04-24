import os
import subprocess
import shutil

# -------------------------------------------------
# Anchor to script location
# -------------------------------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

BUILD_DIR = os.path.join(BASE_DIR, "build", "gcc_single")
EXE_PATH = os.path.join(BUILD_DIR, "hammer_ironclib.exe")

# -------------------------------------------------
# Optional clean
# -------------------------------------------------
def clean():
    shutil.rmtree(BUILD_DIR, ignore_errors=True)

clean()

os.makedirs(BUILD_DIR, exist_ok=True)

# -------------------------------------------------
# Configure CMake (SOURCE OF TRUTH)
# -------------------------------------------------
print("⚙️ Configuring CMake...")

subprocess.check_call([
    "cmake",
    "-S", BASE_DIR,
    "-B", BUILD_DIR,
    "-G", "Ninja",              # or "MinGW Makefiles"
    "-DCMAKE_BUILD_TYPE=Release"
])

# -------------------------------------------------
# Build
# -------------------------------------------------
print("🔨 Building...")

subprocess.check_call([
    "cmake",
    "--build", BUILD_DIR
])

# -------------------------------------------------
# Run
# -------------------------------------------------
print("🚀 Running...\n")

if not os.path.exists(EXE_PATH):
    raise FileNotFoundError(f"Executable not found: {EXE_PATH}")

result = subprocess.run(EXE_PATH)

print(f"\n🏁 Exit code: {result.returncode}")