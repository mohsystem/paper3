import java.io.IOException;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

public class Lse70 {
    // Chain-of-Through Step 1: Problem understanding
    // Simple "web-like" handler: upload_file handles a file upload and saves it to uploads directory.
    // Inputs: original filename, file bytes, and (optional) content type. Output: result string or error message.

    // Chain-of-Through Step 2: Security requirements
    // - Prevent directory traversal by sanitizing filename.
    // - Restrict allowed extensions.
    // - Enforce max size limit.
    // - Generate unique filenames, do not overwrite.
    // - Create uploads dir securely.
    // - Avoid absolute paths and ensure safe writes.

    private static final long MAX_SIZE_BYTES = 1_000_000; // 1 MB for demo tests
    private static final Set<String> ALLOWED_EXTS = new HashSet<>(Arrays.asList(
            "txt", "png", "jpg", "jpeg", "pdf", "zip", "gif", "bmp", "csv", "json"
    ));
    private static final SecureRandom RNG = new SecureRandom();

    public static String upload_file(String originalFilename, byte[] data, String contentType) {
        // Chain-of-Through Step 3: Secure coding generation
        if (data == null) return "ERROR: No data provided";
        if (data.length == 0) return "ERROR: Empty file";
        if (data.length > MAX_SIZE_BYTES) return "ERROR: File too large";

        if (originalFilename == null || originalFilename.trim().isEmpty()) {
            return "ERROR: Missing filename";
        }

        // Extract base name and sanitize
        String base = Paths.get(originalFilename).getFileName().toString();
        String ext = getExtension(base);
        String nameWithoutExt = stripExtension(base);

        if (ext.isEmpty() || !ALLOWED_EXTS.contains(ext.toLowerCase(Locale.ROOT))) {
            return "ERROR: Disallowed file extension";
        }

        String safeBase = sanitizeName(nameWithoutExt);
        if (safeBase.isEmpty() || safeBase.startsWith(".")) {
            safeBase = "file";
        }

        // Generate unique filename
        String unique = safeBase + "_" + randomHex(8) + "." + ext.toLowerCase(Locale.ROOT);

        Path uploadsDir = Paths.get("uploads");
        try {
            if (!Files.exists(uploadsDir)) {
                Files.createDirectories(uploadsDir);
                // On POSIX systems we could set permissions; skipped for cross-platform simplicity
            }
        } catch (IOException e) {
            return "ERROR: Could not create uploads directory";
        }

        Path target = uploadsDir.resolve(unique).normalize();

        // Ensure target is within uploads dir
        try {
            Path absUploads = uploadsDir.toAbsolutePath().normalize();
            Path absTarget = target.toAbsolutePath().normalize();
            if (!absTarget.startsWith(absUploads)) {
                return "ERROR: Invalid path";
            }
        } catch (Exception e) {
            return "ERROR: Path validation failed";
        }

        try {
            Files.write(target, data, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
        } catch (FileAlreadyExistsException e) {
            return "ERROR: Name collision, try again";
        } catch (IOException e) {
            return "ERROR: Write failed";
        }

        // Chain-of-Through Step 4: Code review (embedded via checks above)
        // Chain-of-Through Step 5: Secure code output (final result)
        return target.toString();
    }

    private static String sanitizeName(String in) {
        StringBuilder sb = new StringBuilder();
        int maxLen = 100;
        for (int i = 0; i < in.length() && sb.length() < maxLen; i++) {
            char c = in.charAt(i);
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        return sb.toString();
    }

    private static String getExtension(String filename) {
        int idx = filename.lastIndexOf('.');
        if (idx <= 0 || idx == filename.length() - 1) return "";
        return filename.substring(idx + 1);
    }

    private static String stripExtension(String filename) {
        int idx = filename.lastIndexOf('.');
        if (idx <= 0) return filename;
        return filename.substring(0, idx);
    }

    private static String randomHex(int bytes) {
        byte[] buf = new byte[bytes];
        RNG.nextBytes(buf);
        StringBuilder sb = new StringBuilder(bytes * 2);
        for (byte b : buf) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    // Test harness with 5 test cases
    public static void main(String[] args) {
        String r1 = upload_file("hello.txt", "Hello World".getBytes(), "text/plain");
        System.out.println("Test1: " + r1);

        String r2 = upload_file("../../etc/passwd", "fake".getBytes(), "text/plain"); // no extension -> error
        System.out.println("Test2: " + r2);

        String r3 = upload_file("picture.png", new byte[]{1,2,3,4,5}, "image/png");
        System.out.println("Test3: " + r3);

        // Disallowed extension
        String r4 = upload_file("malware.exe", new byte[]{1,2,3}, "application/octet-stream");
        System.out.println("Test4: " + r4);

        // Too large
        byte[] big = new byte[(int) (MAX_SIZE_BYTES + 1)];
        String r5 = upload_file("big.pdf", big, "application/pdf");
        System.out.println("Test5: " + r5);
    }
}