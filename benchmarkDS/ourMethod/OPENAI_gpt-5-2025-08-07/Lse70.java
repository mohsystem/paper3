import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.channels.ReadableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Locale;

public class Lse70 {

    private static final long MAX_UPLOAD_SIZE = 5L * 1024 * 1024; // 5 MiB
    private static final int MAX_FILENAME_LEN = 100;
    private static final SecureRandom RNG = new SecureRandom();

    public static Path upload_file(Path baseDir, String filename, InputStream bodyStream, long contentLength) throws IOException {
        if (baseDir == null || filename == null || bodyStream == null) {
            throw new IOException("Invalid input");
        }

        // Normalize and ensure baseDir exists and is a directory
        Path realBase;
        try {
            if (!Files.exists(baseDir)) {
                Files.createDirectories(baseDir);
            }
            realBase = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
            if (!Files.isDirectory(realBase, LinkOption.NOFOLLOW_LINKS)) {
                throw new IOException("Base directory is not a directory");
            }
        } catch (IOException e) {
            throw new IOException("Base directory error", e);
        }

        // Validate filename
        String sanitized = sanitizeFilename(filename);
        if (sanitized == null) {
            throw new IOException("Invalid filename");
        }

        // Enforce content length limit (if provided)
        if (contentLength < 0 || contentLength > MAX_UPLOAD_SIZE) {
            throw new IOException("Content length invalid or too large");
        }

        // Generate unique suffix to avoid collisions
        String uniqueSuffix = hexRandom(8) + "-" + Instant.now().toEpochMilli();
        String finalName = sanitized + "-" + uniqueSuffix;
        Path finalPath;
        try {
            finalPath = realBase.resolve(finalName);
            // Ensure final path stays under baseDir
            Path normalizedFinal = finalPath.normalize();
            if (!normalizedFinal.startsWith(realBase)) {
                throw new IOException("Resolved path escapes base directory");
            }
        } catch (InvalidPathException ex) {
            throw new IOException("Invalid path", ex);
        }

        // Create temp file in base directory
        Path tmp = null;
        try {
            tmp = Files.createTempFile(realBase, "tmp-", ".upload");
            // Set restrictive permissions (best-effort; may not be supported on Windows)
            try {
                Files.setPosixFilePermissions(tmp, java.util.Set.of(
                        java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                        java.nio.file.attribute.PosixFilePermission.OWNER_WRITE
                ));
            } catch (UnsupportedOperationException ignored) {
            }

            // Write with limit
            try (ReadableByteChannel in = Channels.newChannel(bodyStream);
                 FileChannel out = FileChannel.open(tmp, StandardOpenOption.WRITE)) {

                long remaining = (contentLength >= 0) ? contentLength : MAX_UPLOAD_SIZE;
                long totalWritten = 0;
                ByteBuffer buffer = ByteBuffer.allocate(64 * 1024);
                while (true) {
                    buffer.clear();
                    int r = in.read(buffer);
                    if (r == -1) break;
                    buffer.flip();
                    if (remaining - r < 0) {
                        throw new IOException("Upload exceeds limit");
                    }
                    while (buffer.hasRemaining()) {
                        int w = out.write(buffer);
                        if (w < 0) throw new IOException("Write failed");
                        totalWritten += w;
                        remaining -= w;
                        if (totalWritten > MAX_UPLOAD_SIZE) throw new IOException("Upload exceeds limit");
                    }
                }
                out.force(true);
            }

            // Atomically move temp to final
            try {
                Files.move(tmp, finalPath, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, finalPath);
            } catch (FileAlreadyExistsException e) {
                // Should not happen due to unique suffix, but fail-closed
                throw new IOException("File exists");
            }
        } catch (IOException e) {
            // Cleanup temp on error
            if (tmp != null) {
                try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
            }
            throw e;
        }

        return finalPath;
    }

    private static String sanitizeFilename(String name) {
        if (name == null) return null;
        String trimmed = name.trim();
        if (trimmed.isEmpty() || trimmed.length() > MAX_FILENAME_LEN) return null;
        // Allow only safe chars
        for (int i = 0; i < trimmed.length(); i++) {
            char c = trimmed.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '-';
            if (!ok) return null;
        }
        if (trimmed.equals(".") || trimmed.equals("..")) return null;
        return trimmed;
    }

    private static String hexRandom(int bytes) {
        byte[] b = new byte[bytes];
        RNG.nextBytes(b);
        StringBuilder sb = new StringBuilder(bytes * 2);
        for (byte v : b) sb.append(String.format(Locale.ROOT, "%02x", v));
        return sb.toString();
    }

    // Simple HTTP server to accept POST /upload with header X-Filename and body bytes
    private static void startServer(Path baseDir) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/upload", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                        send(exchange, 405, "Method Not Allowed\n");
                        return;
                    }
                    Headers headers = exchange.getRequestHeaders();
                    String filename = headers.getFirst("X-Filename");
                    if (filename == null) {
                        send(exchange, 400, "Missing X-Filename header\n");
                        return;
                    }
                    long cl = -1;
                    String clh = headers.getFirst("Content-Length");
                    if (clh != null) {
                        try { cl = Long.parseLong(clh.trim()); } catch (NumberFormatException ignored) {}
                    }
                    if (cl < 0 || cl > MAX_UPLOAD_SIZE) {
                        send(exchange, 413, "Payload too large or missing Content-Length\n");
                        return;
                    }
                    Path saved = upload_file(baseDir, filename, exchange.getRequestBody(), cl);
                    send(exchange, 200, "Saved: " + saved.getFileName().toString() + "\n");
                } catch (IOException ex) {
                    send(exchange, 400, "Upload failed\n");
                }
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("Java server listening on http://127.0.0.1:8080/upload");
    }

    private static void send(HttpExchange ex, int code, String body) throws IOException {
        byte[] data = body.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
        ex.sendResponseHeaders(code, data.length);
        try (var os = ex.getResponseBody()) {
            os.write(data);
        }
    }

    public static void main(String[] args) throws Exception {
        Path uploads = Paths.get("uploads-java");
        Files.createDirectories(uploads);

        // 5 test cases
        try {
            Path r1 = upload_file(uploads, "file.txt", new ByteArrayInputStream("hello".getBytes(StandardCharsets.UTF_8)), 5);
            System.out.println("Test1 OK: " + r1.getFileName());
        } catch (Exception e) {
            System.out.println("Test1 FAIL");
        }
        try {
            byte[] big = new byte[(int) (MAX_UPLOAD_SIZE + 1)];
            Path r2 = upload_file(uploads, "big.bin", new ByteArrayInputStream(big), big.length);
            System.out.println("Test2 FAIL (should be too large): " + r2.getFileName());
        } catch (Exception e) {
            System.out.println("Test2 OK (rejected large)");
        }
        try {
            Path r3 = upload_file(uploads, "safe_name-01.log", new ByteArrayInputStream(new byte[]{1,2,3}), 3);
            System.out.println("Test3 OK: " + r3.getFileName());
        } catch (Exception e) {
            System.out.println("Test3 FAIL");
        }
        try {
            Path r4 = upload_file(uploads, "../bad", new ByteArrayInputStream("x".getBytes(StandardCharsets.UTF_8)), 1);
            System.out.println("Test4 FAIL (bad name accepted): " + r4.getFileName());
        } catch (Exception e) {
            System.out.println("Test4 OK (rejected bad name)");
        }
        try {
            Path r5 = upload_file(uploads, "weird$name.txt", new ByteArrayInputStream("data".getBytes(StandardCharsets.UTF_8)), 4);
            System.out.println("Test5 FAIL (bad chars accepted): " + r5.getFileName());
        } catch (Exception e) {
            System.out.println("Test5 OK (rejected bad chars)");
        }

        // Start the HTTP server
        startServer(uploads);
    }
}