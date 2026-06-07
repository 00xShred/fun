# interp

A tiny Rust interpreter project.

It currently supports:

- numbers
- variables with `let`
- `print` statements
- arithmetic with `+`, `-`, `*`, and `/`
- unary `+` and `-`
- parentheses for grouping

Example:

```text
let x = 4; x + 1
```

Run it with:

```sh
cargo run
```

## Possible Future Features

- reading source code from a file
- booleans and comparisons
- assignments after `let`
- simple control flow like `if` or `while`
