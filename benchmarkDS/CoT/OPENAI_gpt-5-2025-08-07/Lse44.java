import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Objects;

public class Lse44 {
    // Step 1: Problem understanding: Read a requested file from /safe directory securely
    // Step 2: Security requirements: Prevent path traversal, enforce directory boundary, limit file size, avoid symlink escapes
    // Step 3/5: Secure coding generation and output

    private static final Path BASE_DIR = Paths.get("/safe");
    private static final long MAX_BYTES = 5L * 1024 * 1024; // 5 MiB

    public static String readSafeFile(String requestedPath) throws IOException {
        Objects.requireNonNull(requestedPath, "requestedPath cannot be null");

        // Resolve /safe canonical path without following symlinks beyond real base
        Path baseReal = BASE_DIR.toRealPath(LinkOption.NOFOLLOW_LINKS);

        // Normalize requested path: use forward slashes, remove leading slashes to keep it relative
        String rp = requestedPath.replace('\\', '/');
        while (rp.startsWith("/")) {
            rp = rp.substring(1);
        }

        // Resolve target under base and canonicalize, then ensure it remains inside /safe
        Path target = baseReal.resolve(rp).normalize();
        Path targetReal = target.toRealPath(LinkOption.NOFOLLOW_LINKS);

        if (!targetReal.startsWith(baseReal)) {
            throw new SecurityException("Access denied: target escapes base directory");
        }

        if (!Files.isRegularFile(targetReal, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Not a regular file");
        }

        long size = Files.size(targetReal);
        if (size > MAX_BYTES) {
            throw new IOException("File too large");
        }

        byte[] data = Files.readAllBytes(targetReal);
        return new String(data, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        // Step 4: Code review done, testing with 5 test cases
        String[] tests = new String[] {
            "example.txt",
            "subdir/data.txt",
            "../etc/passwd",
            "/safe/../safe/secret.txt",
            "nonexistent.txt"
        };

        for (String t : tests) {
            System.out.println("=== Test: " + t + " ===");
            try {
                String content = readSafeFile(t);
                System.out.println("Read OK. Length: " + content.length());
            } catch (Exception e) {
                System.out.println("Error: " + e.getClass().getSimpleName() + " - " + e.getMessage());
            }
        }
    }
}