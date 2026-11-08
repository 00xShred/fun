# mouse-wiggler

A C virtual mouse that wiggles periodically.

It creates a fake mouse device via `libevdev`/`uinput` and moves it by a small random amount on the X or Y axis every second — useful to keep a machine from going idle.

Build it with:

```sh
cc mouse-wiggler.c -o mouse-wiggler $(pkg-config --cflags --libs libevdev)
```

Run it with:

```sh
./mouse-wiggler
```
