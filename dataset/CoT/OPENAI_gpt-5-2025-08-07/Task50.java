import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task50 {
    // Chain-of-Through process:
    // 1) Problem understanding: Provide a secure backend function to store uploaded files to a server folder and return confirmation.
    // 2) Security requirements: Prevent path traversal, validate filename, limit file size, restrict extensions, ensure safe unique filenames, secure directory and file creation.
    // 3) Secure coding generation: Implement strong sanitization, size checks, and safe file writing with CREATE_NEW to avoid overwriting.
    // 4) Code review: Validate all edge cases and error handling; ensure resources are handled properly.
    // 5) Secure code output: Final hardened implementation below.

    private static final long MAX_SIZE = 10L * 1024 * 1024; // 10 MB
    private static final String UPLOAD_DIR = "uploads";
    private static final Set<String> ALLOWED_EXT = new HashSet<>(Arrays.asList(
            "txt", "pdf", "png", "jpg", "jpeg", "gif", "bin", "dat"
    ));
    private static final SecureRandom RNG = new SecureRandom();

    public static String uploadFile(String originalFileName, byte[] content) throws IOException {
        if (originalFileName == null || originalFileName.trim().isEmpty()) {
            return "ERROR: Invalid file name.";
        }
        if (content == null) {
            return "ERROR: No content provided.";
        }
        if (content.length > MAX_SIZE) {
            return "ERROR: File too large. Max size is " + MAX_SIZE + " bytes.";
        }

        // Create uploads directory securely
        Path uploadPath = Paths.get(UPLOAD_DIR);
        if (!Files.exists(uploadPath)) {
            try {
                // Set directory permissions to 700 (POSIX). If not supported (e.g., Windows), fall back.
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwx------");
                Files.createDirectories(uploadPath, PosixFilePermissions.asFileAttribute(perms));
            } catch (UnsupportedOperationException e) {
                Files.createDirectories(uploadPath);
            }
        }

        String safeName = sanitizeFileName(originalFileName);
        if (safeName == null) {
            return "ERROR: Invalid file name.";
        }

        String ext = getExtension(safeName);
        if (ext.isEmpty() || !ALLOWED_EXT.contains(ext.toLowerCase())) {
            return "ERROR: Disallowed or missing file extension.";
        }

        String base = safeName.substring(0, safeName.length() - ext.length() - 1);
        if (base.isEmpty()) base = "file";
        if (base.length() > 64) base = base.substring(0, 64);

        // Create unique filename and write
        int attempts = 0;
        while (attempts++ < 10) {
            String suffix = randomHex(16);
            String finalName = base + "-" + suffix + "." + ext;
            Path target = uploadPath.resolve(finalName);
            try {
                Files.write(target, content, StandardOpenOption.CREATE_NEW);
                // Try set restrictive permissions if POSIX
                try {
                    Files.setPosixFilePermissions(target, PosixFilePermissions.fromString("rw-------"));
                } catch (UnsupportedOperationException ignored) {
                }
                return "OK: Stored as " + UPLOAD_DIR + "/" + finalName + " (size " + content.length + " bytes).";
            } catch (FileAlreadyExistsException ex) {
                // Retry with a new suffix
            }
        }
        return "ERROR: Could not store file after multiple attempts.";
    }

    private static String sanitizeFileName(String name) {
        String justName = Paths.get(name).getFileName().toString();
        justName = justName.replace("\\", "").replace("/", "");
        if (justName.contains("..")) return null;
        // Replace invalid chars
        String cleaned = justName.replaceAll("[^A-Za-z0-9._-]", "_");
        if (cleaned.isEmpty() || cleaned.length() > 100) return null;
        return cleaned;
    }

    private static String getExtension(String name) {
        int idx = name.lastIndexOf('.');
        if (idx < 0 || idx == name.length() - 1) return "";
        return name.substring(idx + 1);
    }

    private static String randomHex(int len) {
        byte[] buf = new byte[len / 2];
        RNG.nextBytes(buf);
        StringBuilder sb = new StringBuilder(len);
        for (byte b : buf) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        try {
            System.out.println(uploadFile("hello.txt", "Hello, world!".getBytes()));
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
        }

        try {
            System.out.println(uploadFile("../evil.txt", "malicious".getBytes()));
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
        }

        try {
            byte[] pdfBytes = new byte[] {0x25, 0x50, 0x44, 0x46}; // "%PDF"
            System.out.println(uploadFile("report.pdf", pdfBytes));
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
        }

        try {
            byte[] big = new byte[(int) (MAX_SIZE + 1)];
            System.out.println(uploadFile("image.png", big));
        } catch (OutOfMemoryError e) {
            System.out.println("Exception: OutOfMemory");
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
        }

        try {
            System.out.println(uploadFile("bad?.txt", "content".getBytes()));
        } catch (Exception e) {
            System.out.println("Exception: " + e.getMessage());
        }
    }
}