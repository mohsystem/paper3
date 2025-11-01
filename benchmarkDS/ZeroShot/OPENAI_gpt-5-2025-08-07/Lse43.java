import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;
import java.util.List;

public class Lse43 {

    // Reads a file from /safe securely. Returns file content as a UTF-8 String.
    public static String readFromSafeDir(String filename) throws IOException {
        if (filename == null || filename.isEmpty() || filename.indexOf('\0') >= 0) {
            throw new IllegalArgumentException("Invalid filename");
        }

        Path safeBase = Paths.get("/safe").toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path userPath = Paths.get(filename);

        if (userPath.isAbsolute()) {
            throw new SecurityException("Absolute paths are not allowed");
        }

        Path target = safeBase.resolve(userPath).normalize();

        if (!target.startsWith(safeBase)) {
            throw new SecurityException("Path escapes safe directory");
        }

        // Disallow symlinks in the path (including final component)
        Path rel = safeBase.relativize(target);
        Path cur = safeBase;
        for (Path part : rel) {
            cur = cur.resolve(part);
            if (Files.isSymbolicLink(cur)) {
                throw new SecurityException("Symlinks are not allowed in path");
            }
        }

        if (!Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Not a regular file");
        }

        long maxBytes = 10L * 1024 * 1024; // 10 MB
        long size = Files.size(target);
        if (size > maxBytes) {
            throw new IOException("File too large");
        }

        byte[] data = Files.readAllBytes(target);
        return new String(data, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        List<String> tests = Arrays.asList(
                "example.txt",
                "../etc/passwd",
                "/etc/passwd",
                "subdir/notes.txt",
                "symlink_to_secret"
        );

        for (int i = 0; i < tests.size(); i++) {
            String t = tests.get(i);
            System.out.println("Test " + (i + 1) + " (" + t + "):");
            try {
                String content = readFromSafeDir(t);
                String preview = content.length() > 200 ? content.substring(0, 200) + "..." : content;
                System.out.println(preview);
            } catch (Exception e) {
                System.out.println("Error: " + e.getClass().getSimpleName() + ": " + e.getMessage());
            }
            System.out.println();
        }
    }
}