import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;

public class Task65 {
    private static final long MAX_SIZE = 5L * 1024 * 1024; // 5MB

    public static String readFileSecure(String filename) {
        if (!isValidFilename(filename)) {
            System.err.println("Invalid filename format.");
            return null;
        }
        Path path = Paths.get(filename).normalize();

        try {
            if (!Files.exists(path)) {
                System.err.println("File does not exist.");
                return null;
            }
            if (!Files.isReadable(path)) {
                System.err.println("File is not readable.");
                return null;
            }
            BasicFileAttributes attrs = Files.readAttributes(path, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                System.err.println("Not a regular file.");
                return null;
            }
            long size = attrs.size();
            if (size > MAX_SIZE) {
                System.err.println("File too large.");
                return null;
            }
            byte[] data = Files.readAllBytes(path);
            return new String(data, StandardCharsets.UTF_8);
        } catch (IOException e) {
            System.err.println("I/O error: " + e.getMessage());
            return null;
        }
    }

    private static boolean isValidFilename(String filename) {
        if (filename == null) return false;
        String s = filename.trim();
        if (s.isEmpty() || s.length() > 255) return false;
        if (s.equals(".") || s.equals("..")) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < 32 || c == 127) return false; // control chars
            switch (c) {
                case '<': case '>': case ':': case '"':
                case '/': case '\\': case '|': case '?': case '*':
                    return false;
                default:
                    // ok
            }
        }
        return true;
    }

    private static void writeSample(String name, String content) {
        try {
            Files.write(Paths.get(name), content.getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        } catch (IOException e) {
            System.err.println("Failed to write sample file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Prepare sample resources
        writeSample("testfile.txt", "Hello from Task65!\nThis is a secure read test.\n");
        try {
            Files.createDirectories(Paths.get("testdir"));
        } catch (Exception ignored) {}

        // 5 test cases
        String[] tests = new String[] {
                "testfile.txt",       // valid
                "nonexistent.txt",    // not exist
                "bad|name.txt",       // invalid char
                "../secret.txt",      // path traversal attempt (invalid)
                "testdir"             // a directory
        };

        for (int i = 0; i < tests.length; i++) {
            String fname = tests[i];
            System.out.println("=== Java Test " + (i + 1) + ": " + fname + " ===");
            String data = readFileSecure(fname);
            if (data != null) {
                System.out.println("Content length: " + data.length());
                System.out.println(data);
            } else {
                System.out.println("Read failed.");
            }
        }

        // If a command-line argument is provided, try to read it as per the prompt.
        if (args.length > 0) {
            String res = readFileSecure(args[0]);
            if (res != null) {
                System.out.println("=== Java Arg Read ===");
                System.out.println(res);
            } else {
                System.out.println("Could not read file from argument.");
            }
        }
    }
}