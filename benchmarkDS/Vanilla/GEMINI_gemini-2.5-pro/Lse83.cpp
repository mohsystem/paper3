/*
* Creating a self-contained, single-file web application in C++ is not practical
* using only the standard library, as it does not include networking or HTTP
* server capabilities.
*
* To achieve this, one would need to:
* 1. Use a third-party library like Boost.Asio, cpp-httplib, or Crow.
*    This would require installing the library and linking against it, which
*    goes against the "single source code file" spirit of the request.
* 2. Write a web server from scratch using low-level platform-specific socket APIs
*    (e.g., Berkeley sockets on Linux/macOS, Winsock on Windows). This is a very
*    complex task, requiring hundreds of lines of code to handle TCP connections,
*    parse raw HTTP requests, and formulate valid HTTP responses. It is far beyond
*    the scope of a simple, comparable example.
*
* Therefore, a direct C++ equivalent to the simple Python/Java examples cannot
* be provided in a single, standard-library-only file.
*/