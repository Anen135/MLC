# Quick Start

This guide compiles a small MLC program into Mindustry Logic.

## 1. Create a Source File

Create `hello.mlog`:

```mlog
#define SCREEN message1

loop:
    print "Hello from MLC\n"
    printflush SCREEN
    jump loop always 0 0
```

## 2. Compile It

```bash
MLC.exe hello.mlog hello.out.mlog
```

The output file contains plain MLog with labels resolved and macros expanded.

## 3. Paste Into Mindustry

Open a logic processor, import or paste the generated `hello.out.mlog`, and link a display block named `message1`.

## Project Layout

A practical project usually looks like this:

```text
logic/
  reactor.mlog
  defense.mlog
libs/
  display.mlog
  sensor.mlog
  control.mlog
```

Use `#include` when a source file needs shared helpers:

```mlog
#include "libs/display.mlog"

prtl("Ready")
flsh(message1)
```

## Next Steps

- Use [object macros](language.md#object-macros) for constants and linked blocks.
- Use [function macros](language.md#function-macros) for repeated instruction groups.
- Use [standard library helpers](standard-library.md) for common display, sensor, and control operations.
