# Examples

## Reactor Safety Monitor

This program disables a reactor when cryofluid drops below a configured threshold and writes status to a message display.

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

## Inventory Alarm

```mlog
#define CONTAINER vault1
#define SCREEN message1
#define MAX_ITEMS 1000

loop:
    sensor total CONTAINER @totalItems

    print "Items: "
    print total
    print "\n"

    jump full greaterThanEq total MAX_ITEMS
    print "Status: filling"
    jump flush always 0 0

full:
    print "Status: full"

flush:
    printflush SCREEN
    jump loop always 0 0
```

## Turret Targeting

```mlog
#define TURRET duo1

scan:
    radar enemy any any distance TURRET 1 target
    jump idle equal target null

    sensor tx target @x
    sensor ty target @y
    control shoot TURRET tx ty 1 0
    jump scan always 0 0

idle:
    control shoot TURRET 0 0 0 0
    jump scan always 0 0
```
