# Standard Library

The standard library contains small macros for common Mindustry Logic tasks. The exact file names may vary by project, but the modules below describe the intended public helpers.

## Display Module

Usually provided as `disp.mlog` or `display.mlog`.

| Macro | Description | Expands to |
| --- | --- | --- |
| `prt(t)` | Print text or value to buffer. | `print t` |
| `prtl(t)` | Print text or value with newline. | `print t`, `print "\n"` |
| `nl()` | Print newline. | `print "\n"` |
| `flsh(s)` | Flush print buffer to display. | `printflush s` |
| `scrw(s, t)` | Print and flush immediately. | `print t`, `printflush s` |
| `drclr(r, g, b)` | Clear drawing surface. | `draw clear r g b 0 0 0` |
| `drcol(r, g, b, a)` | Set draw color. | `draw color r g b a 0 0` |
| `drrect(x, y, w, h)` | Draw rectangle. | `draw rect x y w h 0 0` |
| `drflsh(d)` | Flush drawing buffer. | `drawflush d` |

## Sensor Module

Usually provided as `sens.mlog` or `sensor.mlog`.

| Macro | Description | Expands to |
| --- | --- | --- |
| `sn_tot(r, b)` | Get total items. | `sensor r b @totalItems` |
| `sn_item(r, b, i)` | Get specific item count. | `sensor r b i` |
| `sn_liq(r, b)` | Get total liquids. | `sensor r b @totalLiquids` |
| `sn_cap(r, b)` | Get item capacity. | `sensor r b @itemCapacity` |
| `sn_heat(r, b)` | Get heat level. | `sensor r b @heat` |
| `sn_on(r, b)` | Get enabled state. | `sensor r b @enabled` |
| `sn_hp(r, b)` | Get health. | `sensor r b @health` |
| `rdr_any(r, b)` | Find any enemy. | `radar enemy any any distance b 1 r` |
| `rdr_air(r, b)` | Find flying enemy. | `radar enemy flying any distance b 1 r` |
| `rdr_gnd(r, b)` | Find ground enemy. | `radar enemy ground any distance b 1 r` |

## Control Module

Usually provided as `ctrl.mlog` or `control.mlog`.

| Macro | Description | Expands to |
| --- | --- | --- |
| `enbl(b)` | Enable block. | `control enabled b 1 0 0 0` |
| `dsbl(b)` | Disable block. | `control enabled b 0 0 0 0` |
| `cstat(b, s)` | Set enabled state. | `control enabled b s 0 0 0` |
| `sh_pos(b, x, y)` | Shoot at position. | `control shoot b x y 1 0` |
| `sh_tgt(b, t)` | Shoot at target. | Reads target `@x` and `@y`, then shoots. |
| `sh_idle(b)` | Stop shooting. | `control shoot b 0 0 0 0` |
| `cfg(b, c)` | Configure block. | `control config b c 0 0 0` |

## Logic Module

Usually provided as `logc.mlog` or `logic.mlog`.

| Macro | Description | Expands to |
| --- | --- | --- |
| `jmpif(l, op, a, b)` | Conditional jump. | `jump l op a b` |
| `jmpe(l, a, b)` | Jump if equal. | `jump l equal a b` |
| `jmpne(l, a, b)` | Jump if not equal. | `jump l notEqual a b` |
| `jmp(l)` | Unconditional jump. | `jump l always 0 0` |
| `isFull(r, b)` | Check whether block inventory is full. | Sensors total and capacity, then compares. |
| `isEmpty(r, b)` | Check whether block inventory is empty. | Sensors total, then compares with `0`. |
| `land(r, a, b)` | Logical AND. | `op land r a b` |
| `lor(r, a, b)` | Logical OR. | `op or r a b` |
| `lnot(r, a)` | Logical NOT. | `op equal r a 0` |
