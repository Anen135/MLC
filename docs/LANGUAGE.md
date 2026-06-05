# MLog Language Documentation

MLog (Mindustry Logic) is a compiled domain-specific language for programming Mindustry logic processors. The MLC compiler extends native MLog with text-based labels, C-like macros, and file modularity.

---

## 1. Language Basics

MLog is a low-level stack-based language executed by Mindustry processors. Each line is an instruction that operates on game blocks.

### 1.1 Instruction Format

```
<instruction> <operand1> <operand2> <operand3> ...
```

### 1.2 Core Instructions

| Instruction | Description |
|-------------|-------------|
| `set` | Assign a value to a variable |
| `sensor` | Read block property into variable |
| `control` | Control a block |
| `print` | Append text to print buffer |
| `printflush` | Flush buffer to display |
| `jump` | Conditional jump to label |
| `op` | Perform arithmetic/logical operation |
| `draw` | 2D drawing operations |
| `drawflush` | Flush drawing buffer |
| `radar` | Detect units |
| `end` | Stop execution |

---

## 2. Standard Declarations

### 2.1 Object Macros (`#define`)

Define simple text substitutions:

```
#define <name> <replacement>
```

**Example:**
```mlog
#define DISPLAY message1
#define MAX_ITEMS 100

sensor items DISPLAY @totalItems
jump panic greaterThan items MAX_ITEMS
```

Compiles to:
```mlog
sensor items message1 @totalItems
jump panic greaterThan items 100
```

### 2.2 Function Macros (`#define` with arguments)

Define parameterized macros:

```
#define <name>(<arg1>, <arg2>, ...) <body>
```

**Example:**
```mlog
#define SAFE_DIV(target, a, b) op div target a b
```

Compiles to a multi-line expansion. Use `\n` for newlines in the body.

### 2.3 Include Directives (`#include`)

Include external library files:

```
#include "path/to/library.mlog"
```

Supports both `"quotes"` and `<brackets>` for path delimiters.

---

## 3. Labels and Jumps

### 3.1 Labels

Define a label by ending a line with a colon:

```
<label_name>:
```

**Example:**
```mlog
main_loop:
    sensor s1 switch1 @enabled
    jump main_loop equal s1 0
```

The compiler automatically resolves labels to line indices.

### 3.2 Jump Instruction

```
jump <target> <condition> <value1> <value2>
```

**Conditions:**
- `always` — Unconditional jump
- `equal` — Jump if value1 == value2
- `notEqual` — Jump if value1 != value2
- `greater` — Jump if value1 > value2
- `less` — Jump if value1 < value2
- `greaterThanEq` — Jump if value1 >= value2
- `lessThanEq` — Jump if value1 <= value2

---

## 4. Standard Library

### 4.1 Display Module (`disp.mlog`)

| Macro | Description |
|------|-------------|
| `prt(t)` | Print text to buffer |
| `prtl(t)` | Print text with newline |
| `nl()` | Print newline |
| `flsh(s)` | Flush buffer to display |
| `scrw(s, t)` | Write and flush |
| `drclr(r, g, b)` | Clear drawing |
| `drcol(r, g, b, a)` | Set draw color |
| `drrect(x, y, w, h)` | Draw rectangle |
| `drflsh(d)` | Flush drawing |

### 4.2 Sensor Module (`sens.mlog`)

| Macro | Description |
|------|-------------|
| `sn_tot(r, b)` | Get total items |
| `sn_item(r, b, i)` | Get specific item |
| `sn_liq(r, b)` | Get total liquids |
| `sn_cap(r, b)` | Get capacity |
| `sn_heat(r, b)` | Get heat level |
| `sn_on(r, b)` | Get enabled state |
| `sn_hp(r, b)` | Get health |
| `rdr_any(r, b)` | Find any enemy |
| `rdr_air(r, b)` | Find flying enemy |
| `rdr_gnd(r, b)` | Find ground enemy |

### 4.3 Control Module (`ctrl.mlog`)

| Macro | Description |
|------|-------------|
| `enbl(b)` | Enable block |
| `dsbl(b)` | Disable block |
| `cstat(b, s)` | Set enabled state |
| `sh_pos(b, x, y)` | Shoot at position |
| `sh_tgt(b, t)` | Shoot at target |
| `sh_idle(b)` | Stop shooting |
| `cfg(b, c)` | Configure block |

### 4.4 Logic Module (`logc.mlog`)

| Macro | Description |
|------|-------------|
| `jmpif(l, op, a, b)` | Conditional jump |
| `jmpe(l, a, b)` | Jump if equal |
| `jmpne(l, a, b)` | Jump if not equal |
| `jmp(l)` | Unconditional jump |
| `isFull(r, b)` | Check if full |
| `isEmpty(r, b)` | Check if empty |
| `land(r, a, b)` | Logical AND |
| `lor(r, a, b)` | Logical OR |
| `lnot(r, a)` | Logical NOT |

---

## 5. Complete Example

```mlog
#define REACTOR reactor1
#define SCREEN message1

#define CRYO_SAFE_LEVEL 12

#define CHECK_CRYO(target) sensor target REACTOR @cryofluid
#define SWITCH_REACTOR(state) control enabled REACTOR state 0 0 0

main_loop:
    CHECK_CRYO(cryo_level)
    
    jump emergency_shutdown lessThan cryo_level CRYO_SAFE_LEVEL
    
    SWITCH_REACTOR(1)
    
    print "=== REACTOR STABLE ===\n"
    print "Cryofluid: "
    print cryo_level
    print " units\n"
    print "Status: OPERATIONAL"
    
    jump display_update always 0 0

emergency_shutdown:
    SWITCH_REACTOR(0)
    
    print "!!! WARNING !!!\n"
    print "CRITICAL COOLING FAILURE!\n"
    print "Cryofluid dropped to: "
    print cryo_level
    print "\nREACTOR EMERGENCY SHUTDOWN!"

display_update:
    printflush SCREEN
    jump main_loop always 0 0
```

---

## 6. Compiler Usage

```bash
MLC.exe <input.mlog> <output.mlog> [lib1.mlog lib2.mlog ...]
```

Default libraries (auto-included):
- `./libs/math.mlog`
- `./libs/display.mlog`