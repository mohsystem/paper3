// C does not have a built-in standard library for creating web servers.
// Implementing this functionality would require external libraries such as libmicrohttpd
// or mongoose. A self-contained, single-file solution without dependencies
// would involve complex low-level socket programming (using sockets from sys/socket.h)
// to handle TCP connections, parse HTTP requests manually, and craft HTTP responses.
// This is a non-trivial task and is not practical for a concise example.