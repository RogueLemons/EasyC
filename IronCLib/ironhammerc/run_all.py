import subprocess
import os
import shutil

# -----------------------------
# Paths (portable)
# -----------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_ROOT = os.path.join(BASE_DIR, "build")

# -----------------------------
# Compiler matrix
# -----------------------------
configs = [
    ("gcc", "gcc", "Ninja"),
    ("clang", "clang", "Ninja"),
    ("msvc", None, "Visual Studio 17 2022"),
]

# -----------------------------
# Test matrix (now ONLY metadata)
# CMake decides actual flags
# -----------------------------
tests = [
    ("c89", "-O0"),
    ("c99", "-O2"),
    ("c11", "-O2"),
]

# -----------------------------
# Helpers
# -----------------------------
def run(cmd, cwd=None):
    print("RUN:", " ".join(cmd))
    subprocess.check_call(cmd, cwd=cwd)

def run_exe(exe):
    print("EXEC:", exe)
    result = subprocess.run(exe)
    return result.returncode

def tool_exists(cmd):
    return shutil.which(cmd) is not None

# -----------------------------
# Test runner
# -----------------------------
failures = []

for compiler_name, compiler, generator in configs:

    # Skip missing compilers
    if compiler_name == "gcc" and not tool_exists("gcc"):
        print("⚠ gcc not found, skipping")
        continue

    if compiler_name == "clang" and not tool_exists("clang"):
        print("⚠ clang not found, skipping")
        continue

    for std, opt in tests:

        # -----------------------------
        # Build directory
        # -----------------------------
        build_dir = os.path.join(
            BUILD_ROOT,
            compiler_name,
            std,
            opt.replace("-", "")
        )

        shutil.rmtree(build_dir, ignore_errors=True)

        # -----------------------------
        # Configure CMake (SOURCE OF TRUTH)
        # -----------------------------
        cmake_cmd = [
            "cmake",
            "-S", BASE_DIR,
            "-B", build_dir,
            "-G", generator,
            f"-DCOMPILER_NAME={compiler_name}",
            f"-DC_STD={std}",
            f"-DOPT_LEVEL={opt},"
        ]

        if compiler:
            cmake_cmd.append(f"-DCMAKE_C_COMPILER={compiler}")

        run(cmake_cmd)

        # -----------------------------
        # Build
        # -----------------------------
        build_cmd = ["cmake", "--build", build_dir]

        if compiler_name == "msvc":
            build_cmd += ["--config", "Release"]

        run(build_cmd)

        # -----------------------------
        # Executable path
        # -----------------------------
        if compiler_name == "msvc":
            exe = os.path.join(build_dir, "Release", "hammer_ironclib.exe")
        else:
            exe = os.path.join(build_dir, "hammer_ironclib.exe")

        if not os.path.exists(exe):
            print(f"❌ Missing executable: {exe}")
            failures.append((f"{compiler_name}-{std}-{opt}", -1))
            continue

        # -----------------------------
        # Run test
        # -----------------------------
        code = run_exe(exe)

        key = f"{compiler_name}-{std}-{opt}"

        if code != 0:
            failures.append((key, code))
            print(f"❌ FAIL: {key} (exit {code})")
        else:
            print(f"✔ PASS: {key}")

# -----------------------------
# Summary
# -----------------------------
print("\n=== TEST SUMMARY ===")

if not failures:
    print("ALL TESTS PASSED ✔")
else:
    print("FAILURES DETECTED ❌")
    for name, code in failures:
        print(f"{name} -> exit code {code}")