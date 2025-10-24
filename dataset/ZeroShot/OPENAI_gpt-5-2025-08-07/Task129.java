import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task129 {
    private static final long MAX_FILE_SIZE = 1_048_576; // 1 MB
    private static final Set<String> ALLOWED_EXTS = new HashSet<>(Arrays.asList("txt", "json", "csv"));
    private static final Pattern SAFE_NAME = Pattern.compile("^[A-Za-z0-9_.-]{1,128}$");

    public static String fetchFile(String baseDir, String filename) throws IOException {
        Objects.requireNonNull(baseDir, "baseDir");
        Objects.requireNonNull(filename, "filename");

        if (!isValidFileName(filename)) {
            throw new SecurityException("Invalid filename.");
        }

        Path basePath = Paths.get(baseDir).toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path candidate = basePath.resolve(filename).normalize();

        if (!Files.exists(candidate) || !Files.isRegularFile(candidate, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("File not found or not a regular file.");
        }

        Path realCandidate = candidate.toRealPath(LinkOption.NOFOLLOW_LINKS);
        if (!realCandidate.startsWith(basePath)) {
            throw new SecurityException("Access denied.");
        }

        long size = Files.size(realCandidate);
        if (size > MAX_FILE_SIZE || size > Integer.MAX_VALUE) {
            throw new IOException("File too large.");
        }

        byte[] data = Files.readAllBytes(realCandidate);
        return new String(data, StandardCharsets.UTF_8);
    }

    private static boolean isValidFileName(String name) {
        if (!SAFE_NAME.matcher(name).matches()) return false;
        if (name.startsWith(".")) return false;
        int idx = name.lastIndexOf('.');
        if (idx <= 0 || idx == name.length() - 1) return false;
        String ext = name.substring(idx + 1).toLowerCase(Locale.ROOT);
        return ALLOWED_EXTS.contains(ext);
    }

    public static void main(String[] args) throws Exception {
        Path base = Files.createTempDirectory("task129_java_");
        try {
            // Prepare test files
            Files.write(base.resolve("a.txt"), "Hello from Java".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Files.write(base.resolve("data.json"), "{\"k\": \"v\"}\n".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Files.write(base.resolve("doc.csv"), "c1,c2\n1,2\n".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            // Big file
            byte[] big = new byte[(int) (MAX_FILE_SIZE + 10)];
            Arrays.fill(big, (byte) 'A');
            Files.write(base.resolve("big.txt"), big, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            // 5 test cases
            String[] tests = new String[] {
                "a.txt",               // valid
                "data.json",           // valid
                "../etc/passwd",       // invalid traversal
                "secret.conf",         // invalid extension
                "big.txt"              // too large
            };

            for (String t : tests) {
                try {
                    String content = fetchFile(base.toString(), t);
                    System.out.println("FETCH OK [" + t + "]: " + content.replace("\n", "\\n"));
                } catch (Exception ex) {
                    System.out.println("FETCH FAIL [" + t + "]: " + ex.getMessage());
                }
            }
        } finally {
            // Cleanup best-effort
            try (DirectoryStream<Path> ds = Files.newDirectoryStream(base)) {
                for (Path p : ds) {
                    try { Files.deleteIfExists(p); } catch (Exception ignored) {}
                }
            } catch (Exception ignored) {}
            try { Files.deleteIfExists(base); } catch (Exception ignored) {}
        }
    }
}