# double-pendulum

A double pendulum simulation written in C using [raylib](https://www.raylib.com/). Renders the two connected pendulums at 60 fps with a fading trail. Made with Euler integration using the standard double-pendulum equations of motion

## Build

```sh
gcc -o double-pendulum double-pendulum.c -lraylib -lm
```
