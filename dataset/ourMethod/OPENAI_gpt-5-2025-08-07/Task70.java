import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.List;

public class Task70 {

    private static final long DEFAULT_MAX_BYTES = 10L * 1024 * 1024; // 10MB

    public static String readFileSecure(String baseDir, String requestedPath, long maxBytes) {
        try {
            if (baseDir == null || baseDir.isEmpty() || baseDir.length() > 4096) {
                return "ERROR: Invalid base directory.";
            }
            if (requestedPath == null || requestedPath.isEmpty() || requestedPath.length() > 4096) {
                return "ERROR: Invalid requested path.";
            }

            Path base = Paths.get(baseDir).toAbsolutePath().normalize();
            if (!Files.exists(base)) {
                return "ERROR: Base directory does not exist.";
            }
            if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Base path is not a directory.";
            }
            if (Files.isSymbolicLink(base)) {
                return "ERROR: Base directory must not be a symbolic link.";
            }

            Path userPath = Paths.get(requestedPath);
            if (userPath.isAbsolute()) {
                return "ERROR: Absolute paths are not allowed.";
            }

            Path candidate = base.resolve(userPath).normalize();
            if (!candidate.startsWith(base)) {
                return "ERROR: Path traversal detected.";
            }

            if (!Files.exists(candidate, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: File does not exist.";
            }

            // Ensure no symlinks in the entire path from base to candidate
            Path rel = base.relativize(candidate);
            Path cur = base;
            for (Path part : rel) {
                cur = cur.resolve(part);
                if (Files.isSymbolicLink(cur)) {
                    return "ERROR: Symbolic links are not allowed.";
                }
            }

            if (!Files.isRegularFile(candidate, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Not a regular file.";
            }

            long size = Files.size(candidate);
            if (size > maxBytes) {
                return "ERROR: File too large.";
            }

            try (BufferedInputStream in = new BufferedInputStream(Files.newInputStream(candidate))) {
                ByteArrayOutputStream out = new ByteArrayOutputStream((int)Math.min(size, Integer.MAX_VALUE));
                byte[] buf = new byte[8192];
                long total = 0;
                int r;
                while ((r = in.read(buf)) != -1) {
                    total += r;
                    if (total > maxBytes) {
                        return "ERROR: File grew beyond allowed size during read.";
                    }
                    out.write(buf, 0, r);
                }
                return new String(out.toByteArray(), StandardCharsets.UTF_8);
            }
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static void printTestHeader(String title) {
        System.out.println("---- " + title + " ----");
    }

    public static void main(String[] args) {
        if (args.length >= 2) {
            String base = args[0];
            String req = args[1];
            String result = readFileSecure(base, req, DEFAULT_MAX_BYTES);
            System.out.println(result);
            return;
        }

        // Self-tests (5 cases)
        try {
            Path tmpBase = Files.createTempDirectory("secure_read_base");
            // Ensure base dir is not a symlink and exists
            Path file1 = tmpBase.resolve("file1.txt");
            Path subdir = tmpBase.resolve("sub");
            Files.createDirectories(subdir);
            Path file2 = subdir.resolve("note.txt");

            Files.write(file1, "Hello world from file1".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Files.write(file2, "Nested content".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            // 1) Read simple file
            printTestHeader("Test 1: Read simple file");
            System.out.println(readFileSecure(tmpBase.toString(), "file1.txt", DEFAULT_MAX_BYTES));

            // 2) Read nested file
            printTestHeader("Test 2: Read nested file");
            System.out.println(readFileSecure(tmpBase.toString(), "sub/note.txt", DEFAULT_MAX_BYTES));

            // 3) Attempt traversal
            printTestHeader("Test 3: Path traversal attempt");
            System.out.println(readFileSecure(tmpBase.toString(), "../etc/passwd", DEFAULT_MAX_BYTES));

            // 4) Attempt to read directory
            printTestHeader("Test 4: Read directory (should fail)");
            System.out.println(readFileSecure(tmpBase.toString(), "sub", DEFAULT_MAX_BYTES));

            // 5) Attempt to read symlink
            Path targetOutside = tmpBase.resolve("outside.txt");
            Files.write(targetOutside, "Outside content".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Path link = tmpBase.resolve("link_to_outside");
            String symlinkResult;
            try {
                Files.deleteIfExists(link);
                Files.createSymbolicLink(link, targetOutside.getFileName()); // relative link within base
                printTestHeader("Test 5: Read symlink (should fail)");
                symlinkResult = readFileSecure(tmpBase.toString(), "link_to_outside", DEFAULT_MAX_BYTES);
            } catch (UnsupportedOperationException | IOException | SecurityException se) {
                printTestHeader("Test 5: Symlink not supported or permission denied - skipped");
                symlinkResult = "SKIPPED: " + se.getMessage();
            }
            System.out.println(symlinkResult);

            // Cleanup (best-effort)
            try {
                Files.deleteIfExists(link);
            } catch (Exception ignore) {}
            try {
                Files.deleteIfExists(file1);
                Files.deleteIfExists(file2);
                Files.deleteIfExists(subdir);
                Files.deleteIfExists(targetOutside);
                Files.deleteIfExists(tmpBase);
            } catch (Exception ignore) {}
        } catch (IOException e) {
            System.err.println("Self-test setup failed: " + e.getMessage());
        }
    }
}