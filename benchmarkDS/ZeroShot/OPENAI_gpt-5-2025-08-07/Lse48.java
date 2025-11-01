import java.io.IOException;
import java.nio.file.*;
import java.util.*;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;

public class Lse48 {
    private static final Path BASE_DIR = Paths.get("static", "images").toAbsolutePath().normalize();
    private static final Set<String> ALLOWED_EXT = new HashSet<>(Arrays.asList("png", "jpg", "jpeg", "gif", "bmp", "webp"));
    private static final Pattern SAFE_NAME = Pattern.compile("^[A-Za-z0-9._-]+$");
    private static final long MAX_FILE_SIZE = 5L * 1024 * 1024;

    public static byte[] img(String fileParam) {
        try {
            if (fileParam == null || fileParam.isEmpty()) return null;
            if (!SAFE_NAME.matcher(fileParam).matches()) return null;

            String lower = fileParam.toLowerCase(Locale.ROOT);
            int dot = lower.lastIndexOf('.');
            if (dot < 0) return null;
            String ext = lower.substring(dot + 1);
            if (!ALLOWED_EXT.contains(ext)) return null;

            Path candidate = BASE_DIR.resolve(fileParam).normalize();
            if (!candidate.startsWith(BASE_DIR)) return null;
            if (!Files.isRegularFile(candidate)) return null;

            long size = Files.size(candidate);
            if (size > MAX_FILE_SIZE) return null;

            return Files.readAllBytes(candidate);
        } catch (IOException e) {
            return null;
        }
    }

    private static void ensureTestFiles() throws IOException {
        Files.createDirectories(BASE_DIR);
        Path f1 = BASE_DIR.resolve("test1.png");
        if (!Files.exists(f1)) {
            Files.write(f1, "PNGDATA".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        }
        Path f2 = BASE_DIR.resolve("test2.txt");
        if (!Files.exists(f2)) {
            Files.write(f2, "TEXTDATA".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        }
    }

    public static void main(String[] args) throws Exception {
        ensureTestFiles();

        String[] tests = new String[] {
            "test1.png",          // valid
            "../secret.txt",      // traversal
            "test2.txt",          // invalid extension
            "missing.jpg",        // missing file
            "bad/evil.png"        // path separator
        };

        for (String t : tests) {
            byte[] out = img(t);
            System.out.println(t + " -> " + (out == null ? "null" : String.valueOf(out.length)));
        }
    }
}