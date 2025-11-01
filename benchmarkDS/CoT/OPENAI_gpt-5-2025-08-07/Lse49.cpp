// Note: Standard C++ does not include built-in support for reading ZIP/TAR archives.
// Without external libraries (e.g., libzip, minizip, libarchive), providing a secure unzip implementation
// that opens "archive.zip" and extracts it to "/tmp/unpack" is not feasible here.
// If allowed to use external libraries, one could implement using libarchive with careful path validation.