# ls-btw

A small C version of `ls`.

It lists files in a directory and supports a couple of familiar flags:

- `-a` shows hidden files
- `-l` prints a long listing with permissions, links, owner, group, size, time, and name

Build it with:

```sh
cc ls.c -o lsbtw
```

Run it with:

```sh
./lsbtw
./lsbtw -a
./lsbtw -l
./lsbtw -al dir-path/
```

This was mostly an exercise in using C directory APIs, file metadata, permissions, and basic command-line flags.
