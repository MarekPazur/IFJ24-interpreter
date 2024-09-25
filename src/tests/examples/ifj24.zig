// IFJ24 pro jazyk Zig (v0.13)
const std = @import("std");

fn writef64(f: f64) void {
    std.fmt.formatFloatHexadecimal(f, std.fmt.FormatOptions{}, std.io.getStdOut().writer()) catch unreachable; // unreachable means ERROR: format error during writef64(f)!
}

pub fn write(value: anytype) void { 
    const stdout = std.io.getStdOut().writer();
    switch (@typeInfo(@TypeOf(value))) { // example from https://ziglang.org/documentation/0.13.0
        //.Int => {
        //    return stdout.print("{d}", .{value});
        //},
        .ComptimeFloat => {
            //stdout.print("comptime_float_as_hex",.{}) catch unreachable;
            return std.fmt.formatFloatHexadecimal(@as(f64,value), std.fmt.FormatOptions{}, stdout) catch unreachable;
        },
        .Float => {
            //return writef64(value);
            std.fmt.formatFloatHexadecimal(value, std.fmt.FormatOptions{}, stdout) catch unreachable;
            return;
        },
        .Pointer => {
            return stdout.print("{s}", .{value}) catch unreachable;
        },
        else => {
            return stdout.print("{}", .{value}) catch unreachable;
        },
    }
}

pub fn readstr() ?[]u8 { // vraci null pri EOF ci interni chybe
    const reader = std.io.getStdIn().reader();
    const allocator = std.heap.page_allocator;
    const buffer: []u8 = allocator.alloc(u8, 1024) catch |e| switch (e) {
        error.OutOfMemory => {
            std.debug.print("ERROR: Buffer allocation FAILED!", .{});
            return null;
        },
    };

    const line = (reader.readUntilDelimiterOrEof(
        buffer,
        '\n',
    ) catch |err| switch (err) {
        error.Unexpected, error.InputOutput, error.AccessDenied, error.BrokenPipe, error.SystemResources, error.OperationAborted, error.WouldBlock, error.ConnectionResetByPeer, error.IsDir, error.ConnectionTimedOut, error.NotOpenForReading, error.SocketNotConnected, error.StreamTooLong => {
            std.debug.print("ERROR: Reading from stdin failed (buffer with 1024 bytes)", .{});
            return null;
        },
    }) orelse return null;
    // trim annoying windows-only carriage return character
    if (@import("builtin").os.tag == .windows) {
        return std.mem.trimRight(u8, line, "\r");
    } else {
        return line;
    }
}

pub fn readi32() ?i32 { // vraci null pri interni chybe (chybou jsou i jen bile znaky pred/za)
    // TODO: check https://ziglang.org/documentation/master/std/#std.io.GenericReader.readInt
    const reader = std.io.getStdIn().reader();
    const allocator = std.heap.page_allocator;
    const buffer: []u8 = allocator.alloc(u8, 1024) catch |e| switch (e) {
        error.OutOfMemory => {
            std.debug.print("ERROR: Buffer allocation FAILED!", .{});
            return null;
        },
    };

    const line = (reader.readUntilDelimiterOrEof(
        buffer,
        '\n',
    ) catch |err| switch (err) {
        error.Unexpected, error.InputOutput, error.AccessDenied, error.BrokenPipe, error.SystemResources, error.OperationAborted, error.WouldBlock, error.ConnectionResetByPeer, error.IsDir, error.ConnectionTimedOut, error.NotOpenForReading, error.SocketNotConnected, error.StreamTooLong => {
            std.debug.print("ERROR: Reading from stdin failed (buffer with 100 bytes)", .{});
            return null;
        },
    }) orelse return null;
    // trim annoying windows-only carriage return character
    if (@import("builtin").os.tag == .windows) {
        return std.mem.trimRight(u8, line, "\r");
    } else {
        return std.fmt.parseInt(i32, line, 10) catch return null;
    }
}

pub fn readf64() ?f64 { // vraci null pri interni chybe (chybou jsou i jen bile znaky pred/za)
    const reader = std.io.getStdIn().reader();
    const allocator = std.heap.page_allocator;
    const buffer: []u8 = allocator.alloc(u8, 1024) catch |e| switch (e) {
        error.OutOfMemory => {
            std.debug.print("ERROR: Buffer allocation FAILED!", .{});
            return null;
        },
    };

    const line = (reader.readUntilDelimiterOrEof(
        buffer,
        '\n',
    ) catch |err| switch (err) {
        error.Unexpected, error.InputOutput, error.AccessDenied, error.BrokenPipe, error.SystemResources, error.OperationAborted, error.WouldBlock, error.ConnectionResetByPeer, error.IsDir, error.ConnectionTimedOut, error.NotOpenForReading, error.SocketNotConnected, error.StreamTooLong => {
            std.debug.print("ERROR: Reading from stdin failed (buffer with 100 bytes)", .{});
            return null;
        },
    }) orelse return null;
    // trim annoying windows-only carriage return character
    if (@import("builtin").os.tag == .windows) {
        return std.mem.trimRight(u8, line, "\r");
    } else {
        return std.fmt.parseFloat(f64, line) catch return null;
    }
}

// Create a slice for literal or (comptime) array representing a literal string
pub fn string(s: []const u8) []u8 {
    var aux: []u8 = std.heap.page_allocator.alloc(u8, s.len) catch unreachable; // unreachable means ERROR: allocation failed during string(s)!
    @memcpy(aux[0..s.len], s);
    return aux;
}

pub fn concat(str1: []u8, str2: []u8) []u8 { // deals with slices (vars)
    var result = std.heap.page_allocator.alloc(u8, str1.len + str2.len) catch unreachable;
    @memcpy(result[0..str1.len], str1);
    @memcpy(result[str1.len..], str2);
    return result;
}

pub fn length(s: []u8) i32 {
    return @as(i32, @intCast(s.len));
}

pub fn i2f(i: i32) f64 {
    return @floatFromInt(i);
}

pub fn f2i(f: f64) i32 {
    return @intFromFloat(f);
}

pub fn substring(s: []u8, i: i32, j: i32) ?[]u8 {
    return if (i < 0 or j < 0 or i > j or i >= s.len or j > s.len) null else s[@intCast(i)..@intCast(j)];
}

pub fn ord(s: []u8, i: i32) i32 {
    return if ((s.len > 0) and (i >= 0) and (i < s.len)) @intCast(s[@intCast(i)]) else 0;
}

pub fn chr(i: i32) []u8 {
    const result: []u8 = std.heap.page_allocator.alloc(u8, 1) catch unreachable;
    result[0] = @intCast(@mod(i, 256));
    return result;
}

pub fn strcmp(s1: []u8, s2: []u8) i32 { // 1 <=> s1 > s2, 0 <=> s1==s2, -1 <=> s1 < s2
    var i : i32 = 0;
    const len1 = length(s1);
    const len2 = length(s2);
    var commonMax : i32 = 0;
    if (len1 > len2) {
        commonMax = len2;
    }
    else { 
        commonMax = len1;
    }
    while (i < commonMax) {
        const a1 = ord(s1, i);
        const a2 = ord(s2, i);
        if (a1 < a2) {
            return -1;
        } else 
        { 
            if (a1 > a2) {
              return 1; 
            } else {}
        }
        i += 1;
    }
    if (len1 == len2) { return 0; }
    else {
      if (len1 > len2) { return 1; }
      else { const ret = 0-1; return ret;}
    }
}