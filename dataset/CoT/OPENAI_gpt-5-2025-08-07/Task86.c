/* Intentionally not providing C implementation that downloads and executes arbitrary EXE files.
   Executing downloaded binaries can be unsafe and can enable misuse.
   A safer approach would include strict domain allowlisting, HTTPS enforcement, maximum size limits,
   robust checksum verification (SHA-256), optional code-signing verification, explicit user consent,
   sandboxing, and principle of least privilege. Execution step should remain disabled by default. */