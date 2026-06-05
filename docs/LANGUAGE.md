# Language Reference

MLC source files are line-oriented. Most lines are native Mindustry Logic instructions, with compiler-only declarations layered on top.

## Instruction Format

```mlog
<instruction> <operand1> <operand2> <operand3> ...
```

Whitespace separates instruction parts. Empty lines are ignored.

## Core Instructions

| Instruction | Description |
| --- | --- |
| `set` | Assign a value to a variable. |
| `sensor` | Read a property from a linked block or unit. |
| `control` | Send a command to a linked block. |
| `print` | Append text or a value to the print buffer. |
| `printflush` | Flush the print buffer to a message display. |
| `jump` | Conditionally jump to another instruction. |
| `op` | Perform arithmetic or logical operations. |
| `draw` | Add drawing commands to the draw buffer. |
| `drawflush` | Flush drawing commands to a display. |
| `radar` | Search for units through a turret or radar-capable block. |
| `end` | Stop the processor program. |

## Labels

Labels end with a colon and are resolved by the compiler:

```mlog
main_loop:
    sensor enabled switch1 @enabled
    jump main_loop equal enabled 0
```

The generated output replaces label targets with numeric line indexes.

## Jump Conditions

```mlog
jump <target> <condition> <value1> <value2>
```

| Condition | Meaning |
| --- | --- |
| `always` | Unconditional jump. |
| `equal` | Jump when `value1 == value2`. |
| `notEqual` | Jump when `value1 != value2`. |
| `greater` | Jump when `value1 > value2`. |
| `less` | Jump when `value1 < value2`. |
| `greaterThanEq` | Jump when `value1 >= value2`. |
| `lessThanEq` | Jump when `value1 <= value2`. |

## Object Macros

Object macros define text substitutions:

```mlog
#define DISPLAY message1
#define MAX_ITEMS 100

sensor items DISPLAY @totalItems
jump panic greaterThan items MAX_ITEMS
```

Compiled output:

```mlog
sensor items message1 @totalItems
jump panic greaterThan items 100
```

Object macros are useful for linked block names, constants, resource names, and shared tuning values.

## Function Macros

Function macros accept arguments:

```mlog
#define SAFE_DIV(target, a, b) op div target a b

SAFE_DIV(result, total, count)
```

Use escaped newlines for multi-line expansions:

```mlog
#define PRINTLN(text) print text\nprint "\n"
```

Prefer short macros for common instruction sequences. If a macro hides too much control flow, keep the logic explicit.

## Includes

Use `#include` to load another source or library file:

```mlog
#include "libs/display.mlog"
#include <libs/control.mlog>
```

Both quoted and bracketed paths are supported. Includes are processed before final label resolution, so included files can provide macros used later in the source file.
