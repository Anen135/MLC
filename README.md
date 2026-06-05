# MLC — Mindustry Logic Compiler

A strict compiler and preprocessor for **Mindustry Logic (mlog)**. It extends the native in-game scripting capabilities by introducing support for text-based labels (eliminating the need to manually recalculate line numbers for jumps), C-like macros (`#define`) with argument mapping and multi-line expansion, and file modularity via `#include`.

---

## 📂 Project Structure

* **`src/`** — Compiler source code (C++).
* **`examples/`** — DSL program examples ready for compilation.
* **`test/`** — An isolated workspace and environment for verifying base logic and linking.
* **`build.bat`** — Automated build script for Windows.

---

## 🛠 Building the Compiler

### Windows

The repository comes with a precompiled `MLC.exe` binary. If you wish to build the compiler from source yourself, run the provided batch script (requires **GCC / MinGW** installed and added to your System PATH):

```bash
.\build.bat

```

### Linux

Currently, there is no precompiled binary or automated build script for Linux. Linux users must compile the source code from the `src/` directory manually using `g++` or `clang++`:

```bash
g++ -O3 src/*.cpp -o MLC

```

---

## ⚠ Critical Requirement (Standard Libraries)

By default, the preprocessor **requires** two core libraries to be present in a `libs` directory at the execution path. Without them, the preprocessing stage will throw an error and fail:

1. `math.mlog` — Contains macros for mathematical and logical operations.
2. `display.mlog` — Houses abstractions for printing and flashing text onto displays/message cells.

Example of a correct environment setup before running tests:

```text
Directory: .\test\libs\

Mode                 Length Name
----                 ------ ----
-a---                    83 display.mlog
-a---                    67 math.mlog

```

---

## 🚀 Usage and CLI Signature

The compiler CLI (`MLC.exe`) accepts arguments in a strict positional order:

```bash
MLC.exe <source_file.mlog> <output_file.mlog> [additional_lib1.mlog additional_lib2.mlog ...]

```

### Argument Specification:

1. **First Argument:** Path to the primary source DSL file you want to compile.
2. **Second Argument:** Path and filename of the resulting output (`.mlog`) file, which will be generated and ready to copy directly into a Mindustry processor.
3. **Subsequent Arguments (Optional):** A list of paths to library files that the linker will automatically inject into the header of the assembly context before the preprocessing phase begins.

> 📌 **Current Limitation:** The compiler does not yet support directory-wide lookups for dependencies.
> **Best Practice:** Create a `libs/` folder right next to your primary `.mlog` source file and store all your external or shared modules inside it.

### Example Compilation Command:

```bash
.\MLC.exe .\test\input.mlog .\test\output.mlog .\test\libs\math.mlog .\test\libs\display.mlog

```
