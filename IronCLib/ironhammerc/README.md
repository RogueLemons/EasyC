# IronHammerC Test Suite
This folder contains a small C test harness and build system used to validate IronCLib and IronHammerC portability across compilers and language modes.

It is intentionally minimal and designed to run on a wide range of toolchains and compilers.

## Table of Contents
* [Overview](#ironhammerc-test-suite)
* [Folder Layout](#folder-layout)
* [Build Matrix](#build-matrix)
* [Purpose](#purpose)
* [Test Framework](#test-framework-ic_hammerh)
* [Requirements](#requirements-for-run_allpy)

## Folder layout
- `ic_hammer.h`: Header-only test framework used by all tests.
- `tests/`: Test cases written using the Iron Hammer C API.
- `main.c`: Entry point that registers and runs all tests.
- `cmake/` (or root `CMakeLists.txt`): Build configuration for all supported compilers and standards.
- `run_all.py`: Runs the full compiler matrix (GCC / Clang / MSVC across C99/C11 and optimization levels).
- `run_gcc.py` (optional helper): Convenience script for local single-tool runs.

## Build matrix
Tests are automatically executed across:
- GCC (C99 / C11)
- Clang (C99 / C11)
- MSVC (C99 / C11 where supported)

Each configuration is tested in:
- `-O0` (debug / correctness)
- `-O2` (optimized behavior)

## Purpose
These tests exist to:
- Verify portability across compilers and C standards
- Catch undefined behavior and non-portable assumptions early
- Allow experimentation with CMake and build configurations
- Provide a reproducible test environment

If you modify the build system, you can validate changes by running:

```bash
python run_all.py
```

Furthermore, the cmake and tests and can be reused for other compilers to validate compatibility in other build environments.

## TEST FRAMEWORK (ic_hammer.h)
The framework is header-only and macro-based for maximum portability.

### Define a test
```c
IHC_TEST(my_test) {
    IHC_ASSERT(x == 1);
    IHC_CHECK(y != 0);
}
```

### Register tests
```c
ihc_test_case tests[] = {
    IHC_TEST_ENTRY(my_test),
};
```

### Run tests
```c
IHC_RUN(tests);
IHC_REPORT();
```

### Assertions
```c
IHC_ASSERT(expr) → fatal failure (stops test)
IHC_CHECK(expr)  → non-fatal failure (continues test)
```

### Output hook
Define this macro to control output:
```c
IHC_PRINT(ctx, test, msg, value, has_value)
```
If not defined, output is disabled, but can still make main return number of failures.

### Exit status
```c
return ihc_failures();
```
Returns number of failed tests.

## Requirements (for run_all.py)
To run the full test suite, the following are needed:
- Python 3 (to run run_all.py)
- CMake
- Ninja (optional, used if available)
- GCC and Clang available on PATH
- MSVC via Visual Studio / Build Tools (tested with VS 17 2022) used through the CMake Visual Studio generator