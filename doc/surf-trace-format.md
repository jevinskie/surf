# surf trace format

## Objectives
*insert out-of-three-pick-two joke here*

Minimal:
```c
struct value_change_log {
    uint16_t signal_id;
    Value signal_val;
};
struct tick_log {
    uint64_t tick;
    uint16_t num_changes;
    value_change_log changes[num_changes];
};
```

Minimal alternative:
```c
struct tick_log {
    uint64_t tick;
    uint16_t num_changes;
    uint16_t signal_ids[num_changes];
    Value changes[num_changes];
};
```
