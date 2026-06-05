# MLC Documentation

MLC is a compiler for extended Mindustry Logic programs. It keeps native MLog close to the surface, but adds the parts that make larger processor programs easier to maintain: labels, C-like macros, includes, and reusable library files.

## What MLC Adds

| Feature | Purpose |
| --- | --- |
| Text labels | Write jumps against names instead of numeric line indexes. |
| Object macros | Reuse constants, linked block names, item names, and tuning values. |
| Function macros | Package common instruction sequences behind short calls. |
| Includes | Split shared helpers into library files and reuse them across programs. |
| Standard library | Use ready-made display, sensor, control, and logic helpers. |

## Start Here

1. Install or build `MLC.exe`.
2. Create an input file such as `reactor.mlog`.
3. Compile it into plain Mindustry Logic.
4. Copy the generated output into a Mindustry logic processor.

```bash
MLC.exe reactor.mlog reactor.out.mlog
```

## Minimal Program

```mlog
#define SCREEN message1

main:
    print "MLC is running\n"
    printflush SCREEN
    jump main always 0 0
```

## Documentation Map

- [Quick Start](quick-start.md) shows the shortest path from source file to generated MLog.
- [Language Reference](language.md) documents instructions, labels, macros, and includes.
- [Standard Library](standard-library.md) lists built-in helper modules.
- [Examples](examples.md) contains complete programs you can adapt.
- [Compiler Usage](compiler.md) explains command-line invocation and file layout.

!!! note
    MLC output is regular Mindustry Logic. The compiler features exist only at build time; the game runs the generated instruction list.
