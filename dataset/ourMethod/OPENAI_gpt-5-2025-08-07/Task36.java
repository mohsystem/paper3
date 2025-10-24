import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;

public class Task36 {

    private static boolean isSafeRelativePath(String userPath) {
        if (userPath == null) return false;
        if (userPath.length() == 0 || userPath.length() > 4096) return false;
        if (userPath.indexOf('\u0000') >= 0) return false; // null byte
        Path p = Paths.get(userPath);
        if (p.isAbsolute()) return false;
        for (Path part : p) {
            String s = part.toString();
            if (s.equals(".") || s.equals("..") || s.isEmpty()) return false;
        }
        return true;
    }

    public static String safeReadFile(String baseDir, String userPath) {
        try {
            if (baseDir == null || baseDir.length() == 0 || baseDir.length() > 4096) {
                return "ERROR: Invalid base directory";
            }
            if (!isSafeRelativePath(userPath)) {
                return "ERROR: Invalid file path";
            }

            Path baseReal = Paths.get(baseDir).toRealPath();
            if (!Files.isDirectory(baseReal)) {
                return "ERROR: Base directory does not exist or is not a directory";
            }

            Path candidate = baseReal.resolve(userPath).normalize();

            if (!candidate.startsWith(baseReal)) {
                return "ERROR: Path escapes base directory";
            }

            // Reject if any component is a symlink
            Path rel = baseReal.relativize(candidate);
            Path walk = baseReal;
            for (Path part : rel) {
                walk = walk.resolve(part);
                if (Files.exists(walk, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(walk)) {
                    return "ERROR: Symlinks are not allowed";
                }
            }

            if (!Files.exists(candidate, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: File does not exist";
            }
            if (!Files.isRegularFile(candidate, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Not a regular file";
            }

            long size = Files.size(candidate);
            long maxSize = 5L * 1024L * 1024L; // 5 MB
            if (size > maxSize) {
                return "ERROR: File too large";
            }

            byte[] data = Files.readAllBytes(candidate);
            return new String(data, StandardCharsets.UTF_8);
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static Path setupTestEnv() throws IOException {
        Path base = Files.createTempDirectory("sandbox36_java_");
        Files.writeString(base.resolve("a.txt"), "Hello from Java A\n", StandardCharsets.UTF_8);
        Files.createDirectories(base.resolve("sub"));
        Files.writeString(base.resolve("sub").resolve("b.txt"), "Hello from Java B\n", StandardCharsets.UTF_8);
        Files.createDirectories(base.resolve("dir"));
        try {
            // Create a symlink if supported
            Path link = base.resolve("link_to_a.txt");
            Files.createSymbolicLink(link, Paths.get("a.txt"));
        } catch (Exception ignored) {
            // Symlink creation may fail on some systems; ignore for test
        }
        return base;
    }

    public static void main(String[] args) {
        try {
            Path base = setupTestEnv();
            List<String> tests = new ArrayList<>();
            tests.add("a.txt");          // valid
            tests.add("sub/b.txt");      // valid
            tests.add("../x.txt");       // path traversal
            tests.add("dir");            // not a regular file
            tests.add("link_to_a.txt");  // symlink (should be rejected or not exist)

            for (String t : tests) {
                String out = safeReadFile(base.toString(), t);
                System.out.println(out);
            }
        } catch (IOException e) {
            System.out.println("ERROR: " + e.getMessage());
        }
    }
}