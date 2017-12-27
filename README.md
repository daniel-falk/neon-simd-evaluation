# neon-simd-evaluation
Evaluation of SIMD performance on Raspberry Pi

Should work on Raspberry Pi 2 (v1.1 and v1.2) and Raspberry Pi 3.

Tested on Pi 2, v1.1

Uses a FIR filter implementation to compare plain c and neon compiler intrinsics

## Result

88% speed up. Should be possible to reach better performance.

```
Using method: plain_c
Monotonic time per sample: 377
Process time per sample:   377

Using method: neon
Monotonic time per sample: 201
Process time per sample:   201
```

## TODO
- Analyze disassembler output
- Implement in pure assembler
- More efficient way to sum over vector lanes?
