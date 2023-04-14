# ntpp - NTP Proxy with Custom Offset

ntpp is a simple NTP proxy that listens for NTP queries, forwards them to an
upstream NTP server, applies a specified offset to the response, and sends the
modified response back to the original client.

## Disclaimer

This software is provided "as is" without warranty of any kind. Use at your own
risk.

## Usage

```
ntpp listen\_port upstream\_ntp\_server\_ip upstream\_ntp\_server\_port offset\_seconds
```

See the man page for details.

## Building

As long as you have libc and a C compiler, you can build and install the project
with:

```
make
make install
```

## Features to be Added

1. Allow the use of hostnames and NTP pools instead of only IP addresses.
1. Default to port 123 and use the standard `host:port` syntax for specifying
   the upstream NTP server.
1. Migrate the codebase to the Rust programming language. 😏
