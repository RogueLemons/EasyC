import subprocess
import os
import shutil

# -----------------------------
# Paths (portable)
# -----------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_ROOT = os.path.join(BASE_DIR, "build")

IS_WINDOWS = os.name == "nt"
EXE_NAME = "hammer_ironclib.exe" if IS_WINDOWS else "hammer_ironclib"

# Optional future hook (disabled unless env var is set)
SANITIZERS = os.environ.get("SANITIZERS", "")

# -----------------------------
# Generator helpers
# -----------------------------
def get_generator(preferred):
    if preferred == "Ninja" and shutil.which("ninja"):
        return "Ninja"

    if IS_WINDOWS:
        if shutil.which("ninja"):
            return "Ninja"
        return "MinGW Makefiles"
    else:
        if shutil.which("ninja"):
            return "Ninja"
        return "Unix Makefiles"


def has_visual_studio():
    try:
        test_dir = os.path.join(BASE_DIR, "tmp_vs_check")
        subprocess.run(
            [
                "cmake",
                "-G", "Visual Studio 17 2022",
                "-S", BASE_DIR,
                "-B", test_dir
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        shutil.rmtree(test_dir, ignore_errors=True)
        return True
    except Exception:
        return False


# -----------------------------
# Compiler matrix
# -----------------------------
configs = [
    ("gcc", "gcc", "Ninja"),
    ("clang", "clang", "Ninja"),
    ("msvc", None, "Visual Studio 17 2022"),
]


# -----------------------------
# Test matrix
# -----------------------------
tests = [
    # Debug builds (important for correctness)
    ("c99", "-O0"),
    ("c11", "-O0"),

    # Optimized builds
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
    result = subprocess.run([exe])
    return result.returncode


def tool_exists(cmd):
    return shutil.which(cmd) is not None


# -----------------------------
# Test runner
# -----------------------------
failures = []
successes = []

for compiler_name, compiler, preferred_gen in configs:

    if compiler_name == "gcc" and not tool_exists("gcc"):
        print("gcc not found, skipping")
        continue

    if compiler_name == "clang" and not tool_exists("clang"):
        print("clang not found, skipping")
        continue

    if compiler_name == "msvc":
        if not IS_WINDOWS or not has_visual_studio():
            print("msvc skipped (Visual Studio not available)")
            continue
        generator = "Visual Studio 17 2022"
    else:
        generator = get_generator(preferred_gen)

    for std, opt in tests:

        opt_tag = opt.replace("-", "O")

        key = f"{compiler_name}-{std}-{opt}"

        build_dir = os.path.join(
            BUILD_ROOT,
            compiler_name,
            std,
            opt_tag
        )

        shutil.rmtree(build_dir, ignore_errors=True)

        # -----------------------------
        # Configure
        # -----------------------------
        cmake_cmd = [
            "cmake",
            "-S", BASE_DIR,
            "-B", build_dir,
            "-G", generator,
            f"-DCOMPILER_NAME={compiler_name}",
            f"-DC_STD={std}",
            f"-DOPT_LEVEL={opt}",
        ]

        if compiler:
            cmake_cmd.append(f"-DCMAKE_C_COMPILER={compiler}")

        if SANITIZERS:
            cmake_cmd.append(f"-DSANITIZERS={SANITIZERS}")

        run(cmake_cmd)

        # -----------------------------
        # Build
        # -----------------------------
        build_cmd = ["cmake", "--build", build_dir]

        if generator.startswith("Visual Studio"):
            build_cmd += ["--config", "Release"]

        run(build_cmd)

        # -----------------------------
        # Executable path
        # -----------------------------
        if generator.startswith("Visual Studio"):
            exe = os.path.join(build_dir, "Release", EXE_NAME)
        else:
            exe = os.path.join(build_dir, EXE_NAME)

        if not os.path.exists(exe):
            print(f"Missing executable: {exe}")
            failures.append((key, -1))
            continue

        # -----------------------------
        # Run
        # -----------------------------
        code = run_exe(exe)

        if code != 0:
            failures.append((key, code))
            print(f"FAIL: {key} (exit {code})")
        else:
            successes.append(key)
            print(f"PASS: {key}")


# -----------------------------
# Summary
# -----------------------------
print("\n=== TEST SUMMARY ===")

print("\nSuccessful runs:")
if successes:
    for s in successes:
        print(f"  {s}")
else:
    print("  None")

print("\nFailures:")
if failures:
    for name, code in failures:
        print(f"  {name} -> exit code {code}")
else:
    print("  None")

print(f"\nTotal: {len(successes) + len(failures)}")
print(f"Passed: {len(successes)}")
print(f"Failed: {len(failures)}")