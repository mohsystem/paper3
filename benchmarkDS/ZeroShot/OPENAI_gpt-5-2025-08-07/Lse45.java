import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Lse45 {
    public static String readFileFromSafeDir(String safeDir, String requested) throws IOException {
        if (safeDir == null || requested == null) {
            throw new IllegalArgumentException("Null parameter");
        }
        if (requested.indexOf('\0') >= 0) {
            throw new IllegalArgumentException("Invalid character in requested path");
        }

        Path base = Paths.get(safeDir).toAbsolutePath().normalize();
        Path req = Paths.get(requested);
        if (req.isAbsolute()) {
            throw new SecurityException("Absolute paths are not allowed");
        }

        Path target = base.resolve(req).normalize();

        if (!target.startsWith(base)) {
            throw new SecurityException("Path traversal attempt detected");
        }

        if (!Files.exists(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("File does not exist");
        }
        if (!Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Target is not a regular file");
        }

        byte[] data = Files.readAllBytes(target);
        return new String(data, StandardCharsets.UTF_8);
    }

    private static void createFile(Path path, String content) throws IOException {
        Files.createDirectories(path.getParent() == null ? path.toAbsolutePath().getParent() : path.getParent());
        Files.write(path, content.getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        Path base = Paths.get("safe_dir_java").toAbsolutePath().normalize();
        Files.createDirectories(base);
        createFile(base.resolve("file1.txt"), "Hello from Java file1");
        createFile(base.resolve("sub/inner.txt"), "Inner Java file");
        // Create an outside file for attempted traversal
        Path outside = base.getParent().resolve("outside_java.txt");
        createFile(outside, "Outside Java file");

        String safeDir = base.toString();

        // Test 1: normal file
        try {
            String s1 = readFileFromSafeDir(safeDir, "file1.txt");
            System.out.println("Test1 OK: " + s1);
        } catch (Exception e) {
            System.out.println("Test1 ERROR: " + e.getMessage());
        }

        // Test 2: subdirectory file
        try {
            String s2 = readFileFromSafeDir(safeDir, "sub/inner.txt");
            System.out.println("Test2 OK: " + s2);
        } catch (Exception e) {
            System.out.println("Test2 ERROR: " + e.getMessage());
        }

        // Test 3: traversal attempt
        try {
            String s3 = readFileFromSafeDir(safeDir, "../outside_java.txt");
            System.out.println("Test3 OK (unexpected): " + s3);
        } catch (Exception e) {
            System.out.println("Test3 Expected ERROR: " + e.getMessage());
        }

        // Test 4: absolute path attempt
        try {
            String s4 = readFileFromSafeDir(safeDir, base.resolve("file1.txt").toString());
            System.out.println("Test4 OK (unexpected): " + s4);
        } catch (Exception e) {
            System.out.println("Test4 Expected ERROR: " + e.getMessage());
        }

        // Test 5: missing file
        try {
            String s5 = readFileFromSafeDir(safeDir, "missing.txt");
            System.out.println("Test5 OK (unexpected): " + s5);
        } catch (Exception e) {
            System.out.println("Test5 Expected ERROR: " + e.getMessage());
        }
    }
}