import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.*;
import java.util.regex.*;

public class Task100 {
    private static final long MAX_FILE_SIZE = 5L * 1024L * 1024L; // 5 MiB
    private static final int MAX_PATTERN_LENGTH = 10000;
    private static final int MAX_MATCHES = 10000;

    public static List<String> searchFile(String pattern, String filePath) {
        Objects.requireNonNull(pattern, "pattern");
        Objects.requireNonNull(filePath, "filePath");
        if (pattern.isEmpty() || pattern.length() > MAX_PATTERN_LENGTH) {
            throw new IllegalArgumentException("Invalid pattern length");
        }

        try {
            Path baseDir = Paths.get("").toAbsolutePath().normalize();
            Path p = Paths.get(filePath).toAbsolutePath().normalize();

            if (!p.startsWith(baseDir)) {
                throw new IllegalArgumentException("Path escapes base directory");
            }
            if (Files.isSymbolicLink(p)) {
                throw new IllegalArgumentException("Symbolic links are not allowed");
            }
            if (!Files.isRegularFile(p, LinkOption.NOFOLLOW_LINKS)) {
                throw new IllegalArgumentException("Not a regular file");
            }

            long size = Files.size(p);
            if (size < 0 || size > MAX_FILE_SIZE) {
                throw new IllegalArgumentException("File too large");
            }

            byte[] bytes = Files.readAllBytes(p);
            String content = new String(bytes, StandardCharsets.UTF_8);

            Pattern compiled = Pattern.compile(pattern, Pattern.DOTALL);
            Matcher m = compiled.matcher(content);
            List<String> results = new ArrayList<>();
            int count = 0;
            while (m.find()) {
                results.add(m.group());
                count++;
                if (count >= MAX_MATCHES) {
                    break;
                }
                // Avoid zero-length match infinite loop
                if (m.start() == m.end() && m.end() < content.length()) {
                    m.region(m.end() + 1, content.length());
                }
            }
            return results;
        } catch (PatternSyntaxException e) {
            throw new IllegalArgumentException("Invalid regex: " + e.getMessage(), e);
        } catch (IOException e) {
            throw new IllegalArgumentException("IO error: " + e.getMessage(), e);
        }
    }

    private static void createFile(String name, String content) throws IOException {
        Path p = Paths.get(name).toAbsolutePath().normalize();
        Path baseDir = Paths.get("").toAbsolutePath().normalize();
        if (!p.startsWith(baseDir)) {
            throw new IOException("Refusing to create outside base directory");
        }
        // Try to set restrictive permissions on POSIX systems
        try {
            Set<PosixFilePermission> perms = EnumSet.of(
                    PosixFilePermission.OWNER_READ,
                    PosixFilePermission.OWNER_WRITE
            );
            Files.write(p, content.getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
            Files.setPosixFilePermissions(p, perms);
        } catch (UnsupportedOperationException ex) {
            // Non-POSIX: best effort write
            Files.write(p, content.getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        }
    }

    private static void printResults(List<String> results) {
        System.out.println("Matches: " + results.size());
        for (int i = 0; i < results.size(); i++) {
            String s = results.get(i);
            String safe = s.replace("\r", "\\r").replace("\n", "\\n");
            if (safe.length() > 200) {
                safe = safe.substring(0, 200) + "...";
            }
            System.out.println("[" + i + "] " + safe);
        }
    }

    public static void main(String[] args) {
        if (args.length == 2) {
            try {
                List<String> res = searchFile(args[0], args[1]);
                printResults(res);
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
                System.exit(1);
            }
            return;
        }

        // 5 test cases
        try {
            createFile("test1.txt", "Hello world!\nThis is a test file.");
            createFile("test2.txt", "Numbers: 123 456 789\nEnd.");
            createFile("test3.txt", "First line.\nSecond line.\nThird line.");
            createFile("test4.txt", "Contacts: alice@example.com, bob@test.com");
            createFile("test5.txt", "No matching tokens here.");

            List<String> r1 = searchFile("world", "test1.txt");
            System.out.println("Test1: pattern='world' file='test1.txt'");
            printResults(r1);

            List<String> r2 = searchFile("\\d+", "test2.txt");
            System.out.println("Test2: pattern='\\d+' file='test2.txt'");
            printResults(r2);

            List<String> r3 = searchFile("Second line\\.", "test3.txt");
            System.out.println("Test3: pattern='Second line\\.' file='test3.txt'");
            printResults(r3);

            List<String> r4 = searchFile("[A-Za-z]+@[A-Za-z]+\\.com", "test4.txt");
            System.out.println("Test4: pattern='[A-Za-z]+@[A-Za-z]+\\.com' file='test4.txt'");
            printResults(r4);

            List<String> r5 = searchFile("XYZ", "test5.txt");
            System.out.println("Test5: pattern='XYZ' file='test5.txt'");
            printResults(r5);
        } catch (Exception e) {
            System.out.println("Test error: " + e.getMessage());
            System.exit(1);
        }
    }
}