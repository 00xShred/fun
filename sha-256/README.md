# sha-256

A pure Bash implementation of the SHA-256 hash algorithm.

It reads from a file or stdin and prints the 64-character hex digest. No external tools — the compression function, padding, and round constants are all done in Bash arithmetic.

Run it with:

```sh
./sha-256 file.txt
echo -n "hello" | ./sha-256
```
