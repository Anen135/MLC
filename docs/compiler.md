# Compiler Usage

## Command

```bash
MLC.exe <input.mlog> <output.mlog> [lib1.mlog lib2.mlog ...]
```

| Argument | Description |
| --- | --- |
| `<input.mlog>` | Source file written with MLC extensions. |
| `<output.mlog>` | Generated plain Mindustry Logic file. |
| `[lib...]` | Optional library files to include during compilation. |

## Examples

Compile one file:

```bash
MLC.exe logic/reactor.mlog build/reactor.out.mlog
```

Compile with explicit libraries:

```bash
MLC.exe logic/defense.mlog build/defense.out.mlog libs/display.mlog libs/control.mlog
```

## Default Libraries

The compiler can auto-include project libraries such as:

```text
./libs/math.mlog
./libs/display.mlog
```

Keep default libraries small and stable. Program-specific helpers are usually clearer when imported explicitly with `#include`.

## Build Output

Compiler output should contain only valid Mindustry Logic:

- No `#define` declarations.
- No `#include` directives.
- No text labels.
- Jump targets resolved to numeric instruction indexes.

## Troubleshooting

| Problem | Check |
| --- | --- |
| Unknown macro | Confirm the macro is defined before use or imported through `#include`. |
| Missing include | Check the path relative to the current source file or compiler working directory. |
| Bad jump target | Confirm the label exists and ends with `:`. |
| Mindustry rejects output | Inspect generated lines for unsupported instructions or malformed operands. |
