import java.io.BufferedReader;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Task99 {

    public static List<String> findMatchingLines(String baseDir, String userPath) throws IOException {
        if (baseDir == null || userPath == null) {
            throw new IllegalArgumentException("Base directory and user path must not be null.");
        }
        if (userPath.length() == 0 || userPath.length() > 4096) {
            throw new IllegalArgumentException("Invalid path length.");
        }

        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.exists(base) || !Files.isDirectory(base)) {
            throw new IllegalArgumentException("Base directory does not exist or is not a directory.");
        }

        Path user = Paths.get(userPath);
        if (user.isAbsolute()) {
            throw new IllegalArgumentException("Absolute paths are not allowed.");
        }

        Path resolved = base.resolve(user).normalize();
        if (!resolved.startsWith(base)) {
            throw new IllegalArgumentException("Resolved path escapes the base directory.");
        }

        if (!Files.exists(resolved, LinkOption.NOFOLLOW_LINKS)) {
            throw new IllegalArgumentException("File does not exist.");
        }
        if (Files.isSymbolicLink(resolved)) {
            throw new IllegalArgumentException("Refusing to process symbolic links.");
        }
        if (!Files.isRegularFile(resolved, LinkOption.NOFOLLOW_LINKS)) {
            throw new IllegalArgumentException("Path is not a regular file.");
        }

        Pattern pattern = Pattern.compile("^\\d.*\\.$");
        List<String> matches = new ArrayList<>();

        try (BufferedReader br = Files.newBufferedReader(resolved, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                if (pattern.matcher(line).matches()) {
                    matches.add(line);
                }
            }
        }

        return matches;
    }

    private static void writeFile(Path base, String name, List<String> lines) {
        try {
            Path p = base.resolve(name);
            Files.createDirectories(p.getParent() == null ? base : p.getParent());
            Files.write(p, String.join(System.lineSeparator(), lines).getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private static void printResult(String title, List<String> result) {
        System.out.println("=== " + title + " ===");
        for (String s : result) {
            System.out.println(s);
        }
        if (result.isEmpty()) {
            System.out.println("(no matches)");
        }
    }

    public static void main(String[] args) throws Exception {
        Path base = Files.createTempDirectory("task99_java_base");
        // Test files
        writeFile(base, "t1.txt", List.of(
                "123 start and end.",
                "Nope",
                "9.",
                "0x23.",
                "abc."
        ));
        writeFile(base, "t2.txt", List.of(
                "A1.",
                "1 but no dot",
                "44.",
                "7!"
        ));
        writeFile(base, "t3.txt", List.of(
                "...",
                "1a.",
                "",
                "2."
        ));
        Files.createDirectories(base.resolve("subdir"));
        writeFile(base, "subdir/inner.txt", List.of(
                "777.",
                "x7."
        ));

        // 5 test cases
        try {
            printResult("Test1 t1.txt", findMatchingLines(base.toString(), "t1.txt"));
        } catch (Exception e) {
            System.out.println("Test1 error: " + e.getMessage());
        }

        try {
            printResult("Test2 t2.txt", findMatchingLines(base.toString(), "t2.txt"));
        } catch (Exception e) {
            System.out.println("Test2 error: " + e.getMessage());
        }

        try {
            printResult("Test3 t3.txt", findMatchingLines(base.toString(), "t3.txt"));
        } catch (Exception e) {
            System.out.println("Test3 error: " + e.getMessage());
        }

        try {
            printResult("Test4 subdir (directory should be rejected)", findMatchingLines(base.toString(), "subdir"));
        } catch (Exception e) {
            System.out.println("Test4 error: " + e.getMessage());
        }

        try {
            printResult("Test5 path traversal outside (should be rejected)", findMatchingLines(base.toString(), "../t1.txt"));
        } catch (Exception e) {
            System.out.println("Test5 error: " + e.getMessage());
        }

        // Additional valid nested file
        try {
            printResult("Extra nested file", findMatchingLines(base.toString(), "subdir/inner.txt"));
        } catch (Exception e) {
            System.out.println("Extra error: " + e.getMessage());
        }
    }
}