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

Indexed:
```c
struct tick_log {
    uint64_t tick;
    uint16_t num_changes;
    uint16_t signal_ids[num_changes];
    uint32_t value_bit_offsets[num_changes];
    uint8_t value_bit_buf[];
};
```

```c++
void dump_tick(struct tick_log *t) {
    auto nc = t->num_changes;
    for (auto i = 0; i < num_changes; ++i) {
        auto id = t->signal_ids[i];
        auto voff = t->value_bit_offsets[i];
        Value val = decode_val(t->value_bit_buf, voff);
        fmt::print("v: {}\n", val);
    }
}
```