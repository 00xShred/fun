const std = @import("std");
const Io = std.Io;

const num_chunks: u64 = 10;

const ProgressState = struct {
    total: u64,
    bytes_downloaded: std.atomic.Value(u64) = .init(0),
    finished_chunks: std.atomic.Value(u64) = .init(0),
};

const CountingWriter = struct {
    state: *ProgressState,
    out: std.Io.File.Writer,
    writer: Io.Writer,

    fn init(out: std.Io.File.Writer, state: *ProgressState) CountingWriter {
        return .{
            .state = state,
            .out = out,
            .writer = .{
                .vtable = &.{
                    .drain = drain,
                    .sendFile = sendFile,
                    .flush = flush,
                },
                .buffer = &.{},
            },
        };
    }

    fn drain(w: *Io.Writer, data: []const []const u8, splat: usize) Io.Writer.Error!usize {
        const this: *CountingWriter = @alignCast(@fieldParentPtr("writer", w));
        const n = try this.out.interface.writeSplat(data, splat);
        _ = this.state.bytes_downloaded.fetchAdd(@intCast(n), .monotonic);
        return n;
    }

    fn sendFile(w: *Io.Writer, file_reader: *Io.File.Reader, limit: Io.Limit) Io.Writer.FileError!usize {
        const this: *CountingWriter = @alignCast(@fieldParentPtr("writer", w));
        const n = try this.out.interface.sendFile(file_reader, limit);
        _ = this.state.bytes_downloaded.fetchAdd(@intCast(n), .monotonic);
        return n;
    }

    fn flush(w: *Io.Writer) Io.Writer.Error!void {
        const this: *CountingWriter = @alignCast(@fieldParentPtr("writer", w));
        try this.out.interface.flush();
    }
};

fn format_progress_line(
    buf: []u8,
    total: u64,
    downloaded: u64,
    eta_seconds: u64,
) ![]const u8 {
    const percent: u64 = if (total == 0) 100 else @min(100, (downloaded * 100) / total);
    return std.fmt.bufPrint(buf, "{d}% {d}/{d} bytes eta {d}s", .{
        percent,
        downloaded,
        total,
        eta_seconds,
    });
}

fn estimate_eta_seconds(downloaded: u64, total: u64, elapsed_ns: u64) u64 {
    if (downloaded == 0 or downloaded >= total) return 0;
    const remaining = total - downloaded;
    const seconds = (@as(f64, @floatFromInt(elapsed_ns)) * @as(f64, @floatFromInt(remaining))) /
        @as(f64, @floatFromInt(downloaded)) /
        @as(f64, @floatFromInt(std.time.ns_per_s));
    return @intFromFloat(@max(0.0, seconds));
}

fn fetch_upstream_metadata(client: *std.http.Client, uri: std.Uri) !u64 {
    var req = try client.request(.HEAD, uri, .{
        .headers = .{
            .accept_encoding = .{ .override = "identity" },
        },
    });

    defer req.deinit();
    try req.sendBodiless();
    var response_buff: [8 * 1024]u8 = undefined;
    const response = try req.receiveHead(&response_buff);

    if (response.head.status != .ok and response.head.status != .partial_content) {
        return error.UnexpectedHttpStatus;
    }

    return response.head.content_length orelse error.ContentLengthUnspecified;
}

const ChunkArgs = struct {
    gpa: std.mem.Allocator,
    uri: std.Uri,
    part_name: []const u8,
    start: u64,
    end: u64,
    progress: *ProgressState,
    result: anyerror!void = {},
};

fn download_chunk_worker(args: *ChunkArgs) void {
    args.result = download_chunk(args);
}

fn download_chunk(args: *ChunkArgs) !void {
    defer _ = args.progress.finished_chunks.fetchAdd(1, .release);

    var threaded: Io.Threaded = .init(args.gpa, .{
        .async_limit = .limited(1),
        .concurrent_limit = .limited(2),
    });
    defer threaded.deinit();
    const io = threaded.io();

    var client: std.http.Client = .{ .allocator = args.gpa, .io = io };
    defer client.deinit();

    const file = try std.Io.Dir.cwd().createFile(io, args.part_name, .{});
    defer file.close(io);

    var range_buff: [128]u8 = undefined;
    const range = try std.fmt.bufPrint(&range_buff, "bytes={d}-{d}", .{ args.start, args.end });

    var writer_buff: [64 * 1024]u8 = undefined;
    const file_writer = file.writer(io, &writer_buff);
    var counting_writer = CountingWriter.init(file_writer, args.progress);
    defer counting_writer.writer.flush() catch {};

    const req = try client.fetch(.{
        .location = .{ .uri = args.uri },
        .response_writer = &counting_writer.writer,
        .headers = .{ .accept_encoding = .{ .override = "identity" } },
        .extra_headers = &.{
            .{ .name = "Range", .value = range },
        },
    });

    if (req.status != .partial_content and req.status != .ok) {
        return error.UnexpectedHttpStatus;
    }
}

pub fn main(init: std.process.Init) !void {
    var threaded: Io.Threaded = .init(init.gpa, .{
        .async_limit = .limited(4),
        .concurrent_limit = .limited(8),
    });
    defer threaded.deinit();
    const io = threaded.io();

    const args = try init.minimal.args.toSlice(init.arena.allocator());
    if (args.len < 2) {
        std.debug.print("Usage: {s} <url>\n", .{args[0]});
        return error.MissingUrlArgument;
    }

    const uri = try std.Uri.parse(args[1]);

    var file_name = std.Io.Dir.path.basenamePosix(uri.path.percent_encoded);
    if (file_name.len == 0) file_name = "index.html";

    var client: std.http.Client = .{ .allocator = init.gpa, .io = io };
    defer client.deinit();

    const file_len = try fetch_upstream_metadata(&client, uri);
    const actual_chunks = @min(num_chunks, file_len);
    const chunk_size = (file_len + actual_chunks - 1) / actual_chunks;
    var progress_state = ProgressState{ .total = file_len };

    std.debug.print("Downloading {s} ({d} bytes) in {d} chunks\n", .{ file_name, file_len, actual_chunks });

    var chunk_args = try init.gpa.alloc(ChunkArgs, actual_chunks);
    defer init.gpa.free(chunk_args);

    var part_names = try init.gpa.alloc([]u8, actual_chunks);
    defer {
        for (part_names) |name| init.gpa.free(name);
        init.gpa.free(part_names);
    }

    for (0..actual_chunks) |i| {
        const start = @as(u64, i) * chunk_size;
        const end = @min(start + chunk_size - 1, file_len - 1);
        part_names[i] = try std.fmt.allocPrint(init.gpa, "{s}.part.{d}", .{ file_name, i });
        chunk_args[i] = .{
            .gpa = init.gpa,
            .uri = uri,
            .part_name = part_names[i],
            .start = start,
            .end = end,
            .progress = &progress_state,
        };
    }

    var threads = try init.gpa.alloc(std.Thread, actual_chunks);
    defer init.gpa.free(threads);

    for (0..actual_chunks) |i| {
        threads[i] = try std.Thread.spawn(.{}, download_chunk_worker, .{&chunk_args[i]});
    }

    const progress_start = std.Io.Clock.Timestamp.now(io, .awake);
    var progress_buf: [128]u8 = undefined;
    var erase_buf: [128]u8 = undefined;
    var previous_len: usize = 0;

    while (progress_state.finished_chunks.load(.acquire) < actual_chunks) {
        const downloaded = progress_state.bytes_downloaded.load(.acquire);
        const now = std.Io.Clock.Timestamp.now(io, .awake);
        const elapsed_raw = now.raw.nanoseconds - progress_start.raw.nanoseconds;
        const elapsed_ns: u64 = if (elapsed_raw <= 0) 0 else @intCast(elapsed_raw);
        const eta = estimate_eta_seconds(downloaded, progress_state.total, elapsed_ns);
        const line = try format_progress_line(&progress_buf, progress_state.total, downloaded, eta);
        const pad_len = @min(previous_len -| line.len, erase_buf.len);
        @memset(erase_buf[0..pad_len], ' ');
        std.debug.print("\r{s}{s}", .{ line, erase_buf[0..pad_len] });
        previous_len = line.len;
        try io.sleep(.fromMilliseconds(200), .awake);
    }

    var first_err: ?anyerror = null;
    for (0..actual_chunks) |i| {
        threads[i].join();
        if (first_err == null) {
            chunk_args[i].result catch |err| {
                first_err = err;
            };
        }
    }

    const final_downloaded = progress_state.bytes_downloaded.load(.acquire);
    const final_eta = estimate_eta_seconds(final_downloaded, progress_state.total, 0);
    const final_line = try format_progress_line(&progress_buf, progress_state.total, final_downloaded, final_eta);
    std.debug.print("\r{s}\n", .{final_line});

    if (first_err) |err| {
        for (part_names) |name| std.Io.Dir.cwd().deleteFile(io, name) catch {};
        return err;
    }

    std.debug.print("Assembling {s}...\n", .{file_name});

    const final_file = try std.Io.Dir.cwd().createFile(io, file_name, .{});
    defer final_file.close(io);

    var final_writer_buf: [64 * 1024]u8 = undefined;
    var final_writer = final_file.writer(io, &final_writer_buf);
    defer final_writer.flush() catch {};

    for (part_names) |part_name| {
        const part = try std.Io.Dir.cwd().openFile(io, part_name, .{});
        defer part.close(io);

        var part_reader_buf: [64 * 1024]u8 = undefined;
        var part_reader = part.reader(io, &part_reader_buf);
        _ = try part_reader.streamMode(&final_writer.interface, .unlimited, .streaming);
        std.Io.Dir.cwd().deleteFile(io, part_name) catch {};
    }

    std.debug.print("Done: {s}\n", .{file_name});
}
