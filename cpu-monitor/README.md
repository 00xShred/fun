# cpu-monitor

A terminal CPU monitor that displays real-time per-core usage as progress bars, reading directly from `/proc/stat`.

## Usage

```bash
./cpu
```

Updates every second in an alternate terminal buffer. Press `Ctrl+C` to exit cleanly.

## Output

Each CPU core is shown as a bar with color-coded usage:

- Green — below 30%
- Yellow — 30–70%
- Red — above 70%
