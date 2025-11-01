// Chain-of-Through steps implemented via comments and secure code practices.
// 1) Problem understanding:
//    - Provide a function to read a file only from the /safe directory.
//    - Inputs: filename (string), optionally a base directory.
//    - Output: file contents as a String.
// 2) Security requirements:
//    - Prevent path traversal and directory escape.
//    - Disallow path separators in filename.
//    - Ensure resolved path is inside base directory.
//    - Reject symbolic links.
//    - Limit maximum file size to avoid memory issues.
// 3) Secure coding generation: implemented in readFromBase and helpers.
// 4) Code review: checks for every step, explicit exceptions, try-with-resources.
// 5) Secure code output: final code follows secure patterns below.

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.regex.Pattern;

public class Lse43 {
    private static final String FIXED_BASE_DIR = "/safe";
    private static final long MAX_BYTES = 10L * 1024 * 1024; // 10 MB
    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    // Secure function to read from a specified base directory
    public static String readFromBase(String baseDir, String filename) throws IOException {
        if (baseDir == null || baseDir.isEmpty()) {
            throw new IllegalArgumentException("Base directory must not be empty");
        }
        if (filename == null || filename.isEmpty()) {
            throw new IllegalArgumentException("Filename must not be empty");
        }

        // Validate filename against strict whitelist; no slashes allowed
        if (!FILENAME_PATTERN.matcher(filename).matches()) {
            throw new SecurityException("Invalid filename: only alphanumerics, dot, underscore, and hyphen allowed");
        }

        Path basePath = Paths.get(baseDir).toAbsolutePath().normalize();
        Path target = basePath.resolve(filename).normalize();

        // Ensure the target is within the base directory
        if (!target.startsWith(basePath)) {
            throw new SecurityException("Attempt to access outside base directory");
        }

        // Reject symbolic links to avoid link-based escape
        if (Files.isSymbolicLink(target)) {
            throw new SecurityException("Symbolic links are not allowed");
        }

        // Verify file exists and is a regular file (no symlinks, no directories)
        if (!Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("File does not exist or is not a regular file");
        }

        // Check size
        long size = Files.size(target);
        if (size > MAX_BYTES) {
            throw new IOException("File too large");
        }

        // Read content safely
        try (InputStream in = Files.newInputStream(target, StandardOpenOption.READ);
             ByteArrayOutputStream bos = new ByteArrayOutputStream((int)Math.min(size, Integer.MAX_VALUE))) {
            byte[] buf = new byte[8192];
            long total = 0;
            int r;
            while ((r = in.read(buf)) != -1) {
                total += r;
                if (total > MAX_BYTES) {
                    throw new IOException("File exceeded maximum allowed size while reading");
                }
                bos.write(buf, 0, r);
            }
            return bos.toString(StandardCharsets.UTF_8);
        }
    }

    // Secure function fixed to /safe directory
    public static String readFromSafeDir(String filename) throws IOException {
        return readFromBase(FIXED_BASE_DIR, filename);
    }

    // Simple helper for output
    private static void printResult(String label, RunnableTest test) {
        System.out.println("== " + label + " ==");
        try {
            String result = test.run();
            System.out.println(result == null ? "(null)" : result);
        } catch (Exception e) {
            System.out.println("Error: " + e.getClass().getSimpleName() + ": " + e.getMessage());
        }
    }

    @FunctionalInterface
    interface RunnableTest {
        String run() throws Exception;
    }

    // Main method with 5 test cases
    public static void main(String[] args) throws Exception {
        // Prepare a temporary base directory for controlled tests
        Path tempBase = Files.createTempDirectory("safe_java_tests_");
        Path goodFile = tempBase.resolve("hello.txt");
        Files.write(goodFile, "Hello from Java".getBytes(StandardCharsets.UTF_8));

        // Create a directory with the same name as a "file" to test non-regular
        Files.createDirectories(tempBase.resolve("not_a_file"));

        System.out.println("Base for tests: " + tempBase);

        // Test 1: Valid read
        printResult("Test 1: valid read", () -> readFromBase(tempBase.toString(), "hello.txt"));

        // Test 2: Path traversal attempt (should be blocked)
        printResult("Test 2: traversal blocked", () -> readFromBase(tempBase.toString(), "../etc/passwd"));

        // Test 3: Invalid characters (should be blocked)
        printResult("Test 3: invalid chars blocked", () -> readFromBase(tempBase.toString(), "bad/name.txt"));

        // Test 4: Non-existing file
        printResult("Test 4: non-existing file", () -> readFromBase(tempBase.toString(), "missing.txt"));

        // Test 5: Not a regular file (directory)
        printResult("Test 5: directory instead of file", () -> readFromBase(tempBase.toString(), "not_a_file"));

        // Optional demonstration with fixed /safe (may fail if /safe not present)
        printResult("Demo: fixed /safe read (may fail)", () -> readFromSafeDir("example.txt"));
    }
}