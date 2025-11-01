import java.io.BufferedInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;
import java.util.List;

public class Lse45 {
    // Chain-of-Through process in code generation:
    // 1) Problem understanding:
    //    - Read a requested file from a predefined safe directory only.
    //    - Prevent directory traversal, absolute paths, and symlink escapes.
    //    - Return the file content as a String; return null on error.
    // 2) Security requirements:
    //    - Validate input (non-null, allowed characters, non-absolute).
    //    - Normalize path and ensure it remains within the safe directory.
    //    - Detect and block symlinks in any path component and the target file.
    //    - Limit file size to a safe maximum before reading.
    // 3) Secure coding generation: implemented in readFromSafeDir.
    // 4) Code review: checks include canonicalization, prefix checks, and symlink checks.
    // 5) Secure code output: final method and test cases provided.

    private static final long MAX_SIZE_BYTES = 1_048_576; // 1 MiB
    private static final Path SAFE_DIR = Paths.get(System.getProperty("user.home"), "safe_dir").toAbsolutePath().normalize();

    public static String readFromSafeDir(String requestedPath) {
        // Input validation
        if (requestedPath == null) return null;
        String trimmed = requestedPath.trim();
        if (trimmed.isEmpty()) return null;
        if (trimmed.indexOf('\0') >= 0) return null; // null byte check

        // allow only safe characters: letters, digits, dot, underscore, dash, forward slash
        if (!trimmed.matches("^[A-Za-z0-9._/\\-]+$")) return null;

        try {
            // Ensure safe directory exists and resolve real path
            Files.createDirectories(SAFE_DIR);
            Path safeReal = SAFE_DIR.toRealPath(LinkOption.NOFOLLOW_LINKS);

            // Disallow absolute requested paths
            Path reqPath = Paths.get(trimmed);
            if (reqPath.isAbsolute()) return null;

            // Normalize target path and ensure it stays inside safe dir
            Path target = SAFE_DIR.resolve(reqPath).normalize();

            if (!target.startsWith(SAFE_DIR)) return null;

            // Ensure target exists and is a regular file
            if (!Files.exists(target) || !Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) return null;

            // Symlink checks for each path component inside the safe directory
            Path relative = SAFE_DIR.relativize(target);
            Path cur = SAFE_DIR;
            for (Path part : relative) {
                cur = cur.resolve(part);
                if (Files.isSymbolicLink(cur)) return null;
            }

            // Final real path check to ensure still under safe dir
            Path targetReal = target.toRealPath(LinkOption.NOFOLLOW_LINKS);
            if (!targetReal.startsWith(safeReal)) return null;

            // Size check
            long size = Files.size(targetReal);
            if (size < 0 || size > MAX_SIZE_BYTES) return null;

            // Read file safely
            byte[] data = new byte[(int) size];
            try (BufferedInputStream in = new BufferedInputStream(Files.newInputStream(targetReal, StandardOpenOption.READ))) {
                int offset = 0;
                while (offset < data.length) {
                    int r = in.read(data, offset, data.length - offset);
                    if (r < 0) break;
                    offset += r;
                }
                if (offset != data.length) return null;
            }
            return new String(data, StandardCharsets.UTF_8);
        } catch (IOException e) {
            return null;
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        List<String> tests = Arrays.asList(
                "example.txt",          // valid (if exists)
                "../secret.txt",        // traversal attempt
                "/etc/passwd",          // absolute path attempt
                "subdir/notes.txt",     // valid subdir (if exists)
                "subdir/../../evil"     // traversal attempt via subdir
        );

        for (String t : tests) {
            String result = readFromSafeDir(t);
            System.out.println("Request: " + t);
            if (result != null) {
                System.out.println("Read OK, length=" + result.length());
            } else {
                System.out.println("Read FAILED");
            }
            System.out.println("---");
        }
    }
}