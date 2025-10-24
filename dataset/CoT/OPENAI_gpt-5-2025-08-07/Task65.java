// Chain-of-Through process:
// 1) Problem understanding: Implement a function that reads a file specified via input parameters
//    and also handle command-line provided filename. Validate filename format and existence, then
//    return the file data safely.
// 2) Security requirements: Prevent path traversal, restrict filename to safe characters, ensure the
//    resolved path stays within a base directory, enforce a maximum file size, and handle errors gracefully.
// 3) Secure coding generation: Use canonical paths, regex for filename validation, size checks,
//    and robust exception handling. Avoid reading overly large files.
// 4) Code review: Checked for path traversal (using base.resolve and startsWith), ensured max-size checks,
//    validated input, used UTF-8 decoding, handled all IO exceptions. No external network or unsafe operations.
// 5) Secure code output: Final code below follows above safeguards.

import java.nio.file.*;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.regex.Pattern;

public class Task65 {
    private static final long MAX_BYTES = 1024; // 1 KiB limit for demo
    private static final Pattern SAFE_NAME = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    // Secure file read with validation. Returns file content or an "ERROR: ..." string.
    public static String safeReadFile(String baseDir, String filename) {
        if (baseDir == null || baseDir.isEmpty()) baseDir = ".";
        if (filename == null || !SAFE_NAME.matcher(filename).matches()) {
            return "ERROR: Invalid filename format. Allowed: letters, digits, . _ - (1..255 chars)";
        }
        try {
            Path base = Paths.get(baseDir).toAbsolutePath().normalize();
            Path target = base.resolve(filename).normalize();

            if (!target.startsWith(base)) {
                return "ERROR: Unsafe path detected.";
            }
            if (!Files.exists(target)) {
                return "ERROR: File does not exist.";
            }
            if (!Files.isRegularFile(target)) {
                return "ERROR: Not a regular file.";
            }
            if (!Files.isReadable(target)) {
                return "ERROR: File is not readable.";
            }
            long size = Files.size(target);
            if (size > MAX_BYTES) {
                return "ERROR: File too large (" + size + " bytes). Max allowed: " + MAX_BYTES + " bytes.";
            }
            byte[] data = Files.readAllBytes(target);
            return new String(data, StandardCharsets.UTF_8);
        } catch (IOException e) {
            return "ERROR: I/O failure: " + e.getMessage();
        } catch (SecurityException se) {
            return "ERROR: Security exception: " + se.getMessage();
        } catch (Exception ex) {
            return "ERROR: Unexpected error: " + ex.getMessage();
        }
    }

    // Main with 5 test cases and optional command-line handling
    public static void main(String[] args) throws Exception {
        Path tempDir = Files.createTempDirectory("task65");
        String base = tempDir.toString();

        // Prepare test files
        Files.write(tempDir.resolve("sample1.txt"),
                "Hello, Task65!".getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

        Files.write(tempDir.resolve("sample-2.txt"),
                "Second sample content.".getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

        byte[] big = new byte[1500]; Arrays.fill(big, (byte) 'A');
        Files.write(tempDir.resolve("big.txt"),
                big,
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

        String[] tests = new String[] {
                "sample1.txt",      // valid
                "sample-2.txt",     // valid with hyphen
                "../etc/passwd",    // invalid path traversal attempt (blocked by format)
                "missing.txt",      // non-existent
                "big.txt"           // too large
        };

        for (int i = 0; i < tests.length; i++) {
            String result = safeReadFile(base, tests[i]);
            System.out.println("Test " + (i + 1) + " (" + tests[i] + "):");
            System.out.println(result);
            System.out.println("-----");
        }

        // If a filename is provided via command line, validate and read from current directory.
        if (args != null && args.length > 0) {
            String res = safeReadFile(".", args[0]);
            System.out.println("Command-line read:");
            System.out.println(res);
        }
    }
}