import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse46 {

    private static final Pattern ALLOWED = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");
    private static final Path BASE_DIR = Paths.get("images");

    public static class ImageResult {
        public final int statusCode; // 200, 400, 403, 404, 500
        public final String contentType;
        public final byte[] data;
        public final String message;

        public ImageResult(int statusCode, String contentType, byte[] data, String message) {
            this.statusCode = statusCode;
            this.contentType = contentType;
            this.data = data;
            this.message = message;
        }
    }

    private static String guessContentType(Path file) {
        try {
            String probed = Files.probeContentType(file);
            if (probed != null) return probed;
        } catch (IOException ignored) {}
        String name = file.getFileName().toString().toLowerCase();
        if (name.endsWith(".png")) return "image/png";
        if (name.endsWith(".jpg") || name.endsWith(".jpeg")) return "image/jpeg";
        if (name.endsWith(".gif")) return "image/gif";
        if (name.endsWith(".webp")) return "image/webp";
        if (name.endsWith(".svg")) return "image/svg+xml";
        if (name.endsWith(".txt")) return "text/plain; charset=utf-8";
        return "application/octet-stream";
    }

    public static ImageResult img(String name) {
        // Validate parameter
        if (name == null || name.isEmpty()) {
            return new ImageResult(400, "text/plain; charset=utf-8", null, "Missing 'name' parameter");
        }
        if (!ALLOWED.matcher(name).matches() || name.startsWith(".")) {
            return new ImageResult(400, "text/plain; charset=utf-8", null, "Invalid file name");
        }

        try {
            Path baseReal = BASE_DIR.toAbsolutePath().normalize();
            // Ensure base directory exists
            if (!Files.exists(baseReal)) {
                Files.createDirectories(baseReal);
            }
            Path target = baseReal.resolve(name).normalize();

            // Prevent path traversal
            if (!target.startsWith(baseReal)) {
                return new ImageResult(400, "text/plain; charset=utf-8", null, "Invalid path");
            }

            // Existence and type checks
            if (!Files.exists(target) || Files.isDirectory(target)) {
                return new ImageResult(404, "text/plain; charset=utf-8", null, "Not found");
            }

            // Block symlinks
            if (Files.isSymbolicLink(target)) {
                return new ImageResult(403, "text/plain; charset=utf-8", null, "Forbidden");
            }

            // Extra: ensure the real path is still within base (handles symlinks in parents)
            Path realTarget = target.toRealPath(LinkOption.NOFOLLOW_LINKS);
            if (!realTarget.startsWith(baseReal)) {
                return new ImageResult(403, "text/plain; charset=utf-8", null, "Forbidden");
            }

            byte[] data = Files.readAllBytes(realTarget);
            String ct = guessContentType(realTarget);
            return new ImageResult(200, ct, data, "OK");

        } catch (SecurityException se) {
            return new ImageResult(403, "text/plain; charset=utf-8", null, "Forbidden");
        } catch (IOException ioe) {
            return new ImageResult(500, "text/plain; charset=utf-8", null, "Internal Server Error");
        }
    }

    public static void main(String[] args) {
        try {
            // Prepare test environment: create images directory and a sample file
            Path base = BASE_DIR.toAbsolutePath().normalize();
            Files.createDirectories(base);
            Path sample = base.resolve("hello.txt");
            if (!Files.exists(sample)) {
                Files.write(sample, "Hello world".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            }

            // 5 Test cases
            Map<String, ImageResult> tests = new HashMap<>();
            tests.put("valid existing", img("hello.txt"));        // should be 200
            tests.put("missing param", img(""));                  // 400
            tests.put("path traversal", img("../secret.txt"));    // 400
            tests.put("invalid chars", img("bad/name.png"));      // 400
            tests.put("not found", img("nope.png"));              // 404

            for (Map.Entry<String, ImageResult> e : tests.entrySet()) {
                System.out.println(e.getKey() + " -> status=" + e.getValue().statusCode + ", type=" + e.getValue().contentType + ", bytes=" + (e.getValue().data == null ? 0 : e.getValue().data.length));
            }

        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}