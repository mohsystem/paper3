import java.nio.file.*;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse44 {
    public static String readSafeFile(String requestedPath) throws IOException {
        if (requestedPath == null) {
            throw new IllegalArgumentException("Requested path is null");
        }
        Path base = Paths.get("/safe").toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path candidate = base.resolve(requestedPath).normalize();
        if (!candidate.startsWith(base)) {
            throw new IllegalArgumentException("Access outside of /safe is not allowed");
        }
        if (!Files.exists(candidate) || !Files.isRegularFile(candidate)) {
            throw new IOException("File does not exist or is not a regular file");
        }
        return Files.readString(candidate, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "example.txt",
            "subdir/data.txt",
            "../etc/passwd",
            "/etc/passwd",
            "nonexistent.txt"
        };
        for (String t : tests) {
            try {
                String content = readSafeFile(t);
                System.out.println("[" + t + "] CONTENT:\n" + content);
            } catch (Exception e) {
                System.out.println("[" + t + "] ERROR: " + e.getMessage());
            }
            System.out.println("----");
        }
    }
}