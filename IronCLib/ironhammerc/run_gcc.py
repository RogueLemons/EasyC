import os
import subprocess
import shutil
import sys

# -------------------------------------------------
# Anchor to script location
# -------------------------------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

BUILD_DIR = os.path.join(BASE_DIR, "build", "gcc_single")

# Detect platform-specific executable name
if os.name == "nt":
    EXE_NAME = "hammer_ironclib.exe"
else:
    EXE_NAME = "hammer_ironclib"

EXE_PATH = os.path.join(BUILD_DIR, EXE_NAME)

# -------------------------------------------------
# Optional clean
# -------------------------------------------------
def clean():
    shutil.rmtree(BUILD_DIR, ignore_errors=True)

clean()

os.makedirs(BUILD_DIR, exist_ok=True)

# -------------------------------------------------
# Choose generator (fallback-friendly)
# -------------------------------------------------
def get_cmake_generator():
    # Prefer Ninja if available
    try:
        subprocess.run(["ninja", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return "Ninja"
    except FileNotFoundError:
        pass

    # Windows fallback
    if os.name == "nt":
        return "MinGW Makefiles"

    # Linux/macOS fallback
    return "Unix Makefiles"

GENERATOR = get_cmake_generator()

# -------------------------------------------------
# Configure CMake
# -------------------------------------------------
print("Configuring CMake...")

subprocess.check_call([
    "cmake",
    "-S", BASE_DIR,
    "-B", BUILD_DIR,
    "-G", GENERATOR,
    "-DCMAKE_BUILD_TYPE=Release"
])

# -------------------------------------------------
# Build
# -------------------------------------------------
print("Building...")

subprocess.check_call([
    "cmake",
    "--build", BUILD_DIR
])

# -------------------------------------------------
# Run
# -------------------------------------------------
print("Running...\n")

if not os.path.exists(EXE_PATH):
    raise FileNotFoundError(f"Executable not found: {EXE_PATH}")

# Use list form for better cross-platform handling
result = subprocess.run([EXE_PATH])

print(f"\nExit code: {result.returncode}")