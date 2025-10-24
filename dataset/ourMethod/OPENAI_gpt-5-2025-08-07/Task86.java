import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystemException;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.security.Security;
import java.time.Duration;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

public class Task86 {

    // Main function performing download and execution. Returns a status string.
    public static String downloadAndExecute(String urlStr, String baseDirStr, boolean perform) {
        // Validate inputs
        if (urlStr == null || urlStr.trim().isEmpty()) {
            return "ERROR: URL is empty.";
        }
        if (urlStr.length() > 2048) {
            return "ERROR: URL too long.";
        }
        if (baseDirStr == null || baseDirStr.trim().isEmpty()) {
            return "ERROR: Base directory is empty.";
        }

        URI uri;
        try {
            uri = new URI(urlStr.trim());
        } catch (URISyntaxException e) {
            return "ERROR: Invalid URL syntax.";
        }

        String scheme = uri.getScheme();
        if (scheme == null || !scheme.equalsIgnoreCase("https")) {
            return "ERROR: Only HTTPS URLs are allowed.";
        }
        if (uri.getHost() == null || uri.getHost().length() > 253) {
            return "ERROR: URL must include a valid host.";
        }

        // Sanitize filename derived from URL
        String rawName = uri.getPath();
        if (rawName == null || rawName.isEmpty()) {
            rawName = "download.exe";
        } else {
            int idx = rawName.lastIndexOf('/');
            rawName = (idx >= 0 && idx + 1 < rawName.length()) ? rawName.substring(idx + 1) : rawName;
            if (rawName.isEmpty()) rawName = "download.exe";
        }
        // Allow only alphanumerics, dash, underscore, dot. Must end with .exe (case-insensitive).
        rawName = rawName.replaceAll("[^A-Za-z0-9._-]", "_");
        if (!rawName.toLowerCase(Locale.ROOT).endsWith(".exe")) {
            rawName = rawName + ".exe";
        }
        if (rawName.length() > 128) rawName = rawName.substring(rawName.length() - 128); // keep tail

        // Resolve and validate base directory
        Path baseDir = Paths.get(baseDirStr).toAbsolutePath().normalize();
        try {
            if (Files.exists(baseDir, LinkOption.NOFOLLOW_LINKS)) {
                if (Files.isSymbolicLink(baseDir)) {
                    return "ERROR: Base directory must not be a symlink.";
                }
                if (!Files.isDirectory(baseDir, LinkOption.NOFOLLOW_LINKS)) {
                    return "ERROR: Base path exists but is not a directory.";
                }
            } else {
                Files.createDirectories(baseDir);
            }
        } catch (IOException e) {
            return "ERROR: Unable to prepare base directory: " + e.getMessage();
        }

        // Final destination path
        Path destPath = baseDir.resolve(rawName).normalize();
        if (!destPath.startsWith(baseDir)) {
            return "ERROR: Resolved destination escapes base directory.";
        }

        // Dry run - for tests and safe default
        if (!perform) {
            return "DRY-RUN: Would download " + urlStr + " to " + destPath.toString() + " and execute it on Windows.";
        }

        // Enable revocation checking (best-effort; platform/JRE dependent)
        try {
            Security.setProperty("ocsp.enable", "true");
            System.setProperty("com.sun.net.ssl.checkRevocation", "true");
            System.setProperty("jdk.tls.client.enableStatusStapling", "true");
        } catch (SecurityException ignored) {
            // Best effort only
        }

        // HTTP client
        HttpClient client = HttpClient.newBuilder()
                .followRedirects(HttpClient.Redirect.NORMAL)
                .connectTimeout(Duration.ofSeconds(20))
                .version(HttpClient.Version.HTTP_2)
                .build();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(uri)
                .timeout(Duration.ofSeconds(60))
                .GET()
                .header("User-Agent", "Task86Downloader/1.0")
                .build();

        // Download to a temporary file within base directory
        Path tmp = null;
        long maxBytes = 100L * 1024 * 1024; // 100 MB cap
        long total = 0L;

        try {
            HttpResponse<InputStream> resp = client.send(request, HttpResponse.BodyHandlers.ofInputStream());
            int code = resp.statusCode();
            if (code < 200 || code >= 300) {
                return "ERROR: HTTP status " + code + " received.";
            }

            tmp = Files.createTempFile(baseDir, "dl_", ".tmp");
            try (InputStream in = resp.body();
                 FileChannel ch = FileChannel.open(tmp, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {

                try (FileLock lock = ch.lock()) {
                    byte[] buf = new byte[8192];
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        total += r;
                        if (total > maxBytes) {
                            return "ERROR: Download exceeds maximum allowed size.";
                        }
                        ch.write(java.nio.ByteBuffer.wrap(buf, 0, r));
                    }
                    ch.force(true);
                }
            }

            // Set restrictive permissions if possible
            try {
                // On POSIX systems
                java.util.Set<java.nio.file.attribute.PosixFilePermission> perms =
                        java.util.EnumSet.of(
                                java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                                java.nio.file.attribute.PosixFilePermission.OWNER_WRITE,
                                java.nio.file.attribute.PosixFilePermission.OWNER_EXECUTE
                        );
                Files.setPosixFilePermissions(tmp, perms);
            } catch (UnsupportedOperationException ignored) {
                // On Windows use DOS attributes as needed
                try {
                    Files.setAttribute(tmp, "dos:readonly", false, LinkOption.NOFOLLOW_LINKS);
                } catch (Exception ignored2) {}
            }

            // Atomically move into place
            try {
                Files.move(tmp, destPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, destPath, StandardCopyOption.REPLACE_EXISTING);
            }

            // Ensure final file is a regular file and not a symlink
            if (Files.isSymbolicLink(destPath) || !Files.isRegularFile(destPath, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Final file is not a regular file.";
            }

        } catch (IOException e) {
            try {
                if (tmp != null) Files.deleteIfExists(tmp);
            } catch (IOException ignored) {}
            return "ERROR: Download failed: " + e.getMessage();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            try {
                if (tmp != null) Files.deleteIfExists(tmp);
            } catch (IOException ignored) {}
            return "ERROR: Download interrupted.";
        }

        // Execute only on Windows
        String os = System.getProperty("os.name", "").toLowerCase(Locale.ROOT);
        if (!os.contains("win")) {
            return "OK: Downloaded to " + destPath + " but not executed (non-Windows OS).";
        }

        // Execute the downloaded file safely
        Process proc = null;
        try {
            ProcessBuilder pb = new ProcessBuilder(destPath.toString());
            pb.directory(baseDir.toFile());
            pb.redirectErrorStream(true);
            proc = pb.start();

            boolean finished = proc.waitFor(15, TimeUnit.SECONDS);
            if (!finished) {
                proc.destroyForcibly();
                return "OK: Executed " + destPath + " but terminated due to timeout.";
            }
            int exit = proc.exitValue();
            return "OK: Executed " + destPath + " with exit code " + exit + ".";
        } catch (IOException e) {
            return "ERROR: Execution failed: " + e.getMessage();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            if (proc != null) proc.destroyForcibly();
            return "ERROR: Execution interrupted.";
        }
    }

    public static void main(String[] args) {
        // 5 test cases (all dry-run to avoid side effects)
        String base = System.getProperty("java.io.tmpdir");
        String[] tests = new String[] {
                "https://example.com/vendor/product.exe",
                "http://example.com/insecure.exe",
                "https://example.com/path/../../../evil.exe",
                "https://very-long-host-name-that-is-probably-invalid.example.com/" + "a".repeat(200) + ".exe",
                ""
        };

        for (int i = 0; i < tests.length; i++) {
            String res = downloadAndExecute(tests[i], base, false);
            System.out.println("Test " + (i+1) + ": " + res);
        }
    }
}