import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Lse45 {
    public static String readFileFromSafeDir(String safeDir, String requestedPath) {
        try {
            Path base = Paths.get(safeDir).toAbsolutePath().normalize();
            if (!Files.exists(base)) {
                Files.createDirectories(base);
            }
            Path req = Paths.get(requestedPath);
            if (req.isAbsolute()) {
                return null;
            }
            Path candidate = base.resolve(req).normalize();
            if (!candidate.startsWith(base)) {
                return null;
            }
            if (!Files.exists(candidate) || !Files.isRegularFile(candidate)) {
                return null;
            }
            return Files.readString(candidate, StandardCharsets.UTF_8);
        } catch (Exception e) {
            return null;
        }
    }

    public static void main(String[] args) throws IOException {
        Path base = Paths.get("safe_dir_java").toAbsolutePath().normalize();
        Files.createDirectories(base.resolve("sub"));
        Files.writeString(base.resolve("hello.txt"), "Hello from Java", StandardCharsets.UTF_8);
        Files.writeString(base.resolve("sub").resolve("inner.txt"), "Inner from Java", StandardCharsets.UTF_8);

        String[] tests = new String[] {
            "hello.txt",                         // valid
            "sub/inner.txt",                     // valid in subdir
            "../hello.txt",                      // traversal attempt
            base.resolve("hello.txt").toString(),// absolute path attempt
            "missing.txt"                        // non-existent
        };

        for (String t : tests) {
            String content = readFileFromSafeDir(base.toString(), t);
            System.out.println("Request: " + t + " => " + (content == null ? "(blocked or not found)" : content));
        }
    }
}