# qrcode-generator

A command-line QR code generator written in C. Reads text from stdin and writes a PNG image to `out.png`.

## Dependencies

- [nayuki/QR-Code-generator](https://github.com/nayuki/QR-Code-generator) — `qrcodegen.c` / `qrcodegen.h`
- [nayuki/Tiny-PNG-Output](https://github.com/nayuki/Tiny-PNG-Output) — `TinyPngOut.c` / `TinyPngOut.h`

Both libraries are vendored in the directory.

## Build

```sh
gcc -o generator generator.c qrcodegen.c TinyPngOut.c
```

## Usage

```sh
echo "https://example.com" | ./generator
```
