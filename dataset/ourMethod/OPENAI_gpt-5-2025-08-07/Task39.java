import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

public class Task39 {

    // Validates, resolves, and ensures the path is safe and within baseDir without symlinks
    private static Path validateAndResolve(Path baseDir, String userFile) throws IOException {
        if (userFile == null) {
            throw new IOException("Invalid input: null");
        }
        if (userFile.indexOf('\0') >= 0) {
            throw new IOException("Invalid input: null byte");
        }
        if (userFile.length() == 0 || userFile.length() > 255) {
            throw new IOException("Invalid input length");
        }
        // Allow only safe characters (no spaces or shell metacharacters)
        Pattern allowed = Pattern.compile("^[A-Za-z0-9._/\\\\-]+$");
        if (!allowed.matcher(userFile).matches()) {
            throw new IOException("Invalid characters in filename");
        }
        if (Paths.get(userFile).isAbsolute()) {
            throw new IOException("Path must be relative");
        }

        Path baseReal = baseDir.toRealPath(); // canonical base
        Path candidate = baseReal.resolve(userFile).normalize();

        if (!candidate.startsWith(baseReal)) {
            throw new IOException("Path traversal detected");
        }

        // Disallow any symlink in the path (including intermediate components)
        Path current = baseReal;
        Path rel = baseReal.relativize(candidate);
        for (Path part : rel) {
            current = current.resolve(part);
            if (Files.exists(current, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(current)) {
                throw new IOException("Symlink not allowed: " + current);
            }
        }

        if (!Files.exists(candidate, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("File does not exist");
        }
        if (!Files.isRegularFile(candidate, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Not a regular file");
        }

        return candidate;
    }

    // Executes a system command ("cat" or "more") to display the file's content and returns it
    public static String displayFileUsingCommand(String baseDirStr, String userFile) {
        try {
            Path baseDir = Paths.get(baseDirStr).toAbsolutePath();
            Files.createDirectories(baseDir);

            Path resolved = validateAndResolve(baseDir, userFile);

            boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
            List<String> command = new ArrayList<>();
            if (isWindows) {
                // Use external program "more" to avoid shell built-in
                command = Arrays.asList("more", resolved.toString());
            } else {
                command = Arrays.asList("cat", resolved.toString());
            }

            ProcessBuilder pb = new ProcessBuilder(command);
            pb.redirectErrorStream(true);
            StringBuilder output = new StringBuilder();
            try (BufferedReader br = new BufferedReader(
                    new InputStreamReader(pb.start().getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = br.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }
            int exit = pb.start().waitFor(); // ensure process completed if not already
            if (exit != 0 && output.length() == 0) {
                return "ERROR: Command failed with exit code " + exit;
            }
            return output.toString();
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static void writeFile(Path path, String content) throws IOException {
        Files.createDirectories(path.getParent() == null ? path.toAbsolutePath().getParent() : path.getParent());
        Files.write(path, content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
    }

    public static void main(String[] args) throws Exception {
        String base = "safe_base_dir_task39_java";
        Path basePath = Paths.get(base).toAbsolutePath();
        Files.createDirectories(basePath);

        // Prepare test files
        writeFile(basePath.resolve("file1.txt"), "Hello from Java file1\nLine2\n");
        Files.createDirectories(basePath.resolve("subdir"));
        writeFile(basePath.resolve("subdir/file2.txt"), "Hello from Java subdir file2\n");
        Files.createDirectories(basePath.resolve("emptydir"));

        // Five test cases
        String[] tests = new String[] {
                "file1.txt",                 // valid
                "subdir/file2.txt",          // valid nested
                "../outside.txt",            // traversal attempt
                "nonexistent.txt",           // non-existent
                "subdir"                     // directory, not file
        };

        for (int i = 0; i < tests.length; i++) {
            System.out.println("=== Test " + (i + 1) + ": " + tests[i] + " ===");
            String result = displayFileUsingCommand(base, tests[i]);
            System.out.print(result);
        }
    }
}