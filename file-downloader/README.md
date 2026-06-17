# file-downloader

Parallel file downloader written in Zig. Splits a file into chunks and downloads them concurrently using HTTP Range requests, then assembles them.

## Usage

```
file_downloader <url>
```

Output file is named from the URL path basename (falls back to `index.html`).

## How it works

1. HEAD request to get `Content-Length`
2. Up to 10 chunks downloaded in parallel threads
3. Progress line with `%`, bytes, and ETA updated every 200ms
4. Chunks assembled into final file, part files deleted

## Build

```sh
zig build
./zig-out/bin/file_downloader <url>
```
