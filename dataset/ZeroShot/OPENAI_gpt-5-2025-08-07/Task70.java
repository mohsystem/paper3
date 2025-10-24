import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Task70 {
    // Securely reads a file within the current working directory, max 10MB, UTF-8.
    public static String readFileSecure(String inputPath) {
        if (inputPath == null || inputPath.isEmpty()) {
            return null;
        }
        try {
            Path base = Paths.get(System.getProperty("user.dir")).toRealPath();
            Path canonical = Paths.get(inputPath).toRealPath();
            if (!canonical.startsWith(base)) {
                return null;
            }
            if (!Files.isRegularFile(canonical, LinkOption.NOFOLLOW_LINKS) || !Files.isReadable(canonical)) {
                return null;
            }
            long size = Files.size(canonical);
            long max = 10L * 1024 * 1024; // 10MB limit
            if (size > max) {
                return null;
            }
            byte[] data = Files.readAllBytes(canonical);
            return new String(data, StandardCharsets.UTF_8);
        } catch (IOException e) {
            return null;
        }
    }

    // If args provided: treat as file paths and print contents; else run 5 tests.
    public static void main(String[] args) {
        if (args.length > 0) {
            for (String p : args) {
                String content = readFileSecure(p);
                System.out.println("----- " + p + " -----");
                if (content != null) {
                    System.out.println(content);
                } else {
                    System.out.println("ERROR: Unable to read file securely.");
                }
            }
            return;
        }

        // 5 test cases
        try {
            Path t1 = Paths.get("java_t1.txt");
            Files.writeString(t1, "Java Test 1: Hello World", StandardCharsets.UTF_8);

            Path subDir = Paths.get("java_subdir");
            Files.createDirectories(subDir);
            Path t2 = subDir.resolve("java_t2.txt");
            Files.writeString(t2, "Java Test 2: Inside subdir", StandardCharsets.UTF_8);

            String[] testPaths = new String[] {
                "java_t1.txt",              // valid
                "java_subdir/java_t2.txt",  // valid in subdir
                "java_no_such.txt",         // non-existent
                "java_subdir",              // a directory (not a file)
                "../java_outside.txt"       // attempt to escape base
            };

            for (String p : testPaths) {
                System.out.println("===== Testing: " + p + " =====");
                String content = readFileSecure(p);
                if (content != null) {
                    System.out.println(content);
                } else {
                    System.out.println("ERROR: Unable to read file securely.");
                }
            }
        } catch (IOException e) {
            System.out.println("ERROR: Test setup failed: " + e.getMessage());
        }
    }
}