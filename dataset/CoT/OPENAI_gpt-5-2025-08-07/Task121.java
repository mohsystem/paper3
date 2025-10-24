// Chain-of-Through process in code:
// 1) Problem understanding: Provide a secure file upload utility that accepts a filename and data, validates and stores it safely.
// 2) Security requirements: Enforce size limits, whitelist extensions, magic-byte sniffing, sanitize filenames, prevent traversal, unique naming, safe directory creation, atomic write-then-move, restrictive permissions.
// 3) Secure coding generation: Implement step-by-step with clear validation and safe I/O.
// 4) Code review: Inline comments mark each security control; checks for edge cases and exceptions.
// 5) Secure code output: Final code below applies mitigations identified above.

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;

public class Task121 {

    private static final long MAX_SIZE = 5L * 1024 * 1024; // 5 MB
    private static final Set<String> ALLOWED_EXT = new HashSet<>(Arrays.asList("txt", "png", "jpg", "jpeg", "pdf"));
    private static final SecureRandom RNG = new SecureRandom();

    // Main upload function: validates and stores the file safely. Returns absolute saved path or null on failure.
    public static String uploadFile(String originalFilename, byte[] data, String destDir) {
        try {
            // Basic validations
            if (originalFilename == null || data == null) return null;
            if (data.length == 0 || data.length > MAX_SIZE) return null;

            // Sanitize filename and extract extension
            String safeName = sanitizeFilename(originalFilename);
            String ext = getExtension(safeName);
            if (ext == null || !ALLOWED_EXT.contains(ext)) return null;

            // Content sniffing for allowed types
            if (!contentMatchesExtension(data, ext)) return null;

            // Prepare destination directory
            Path dir = Paths.get(destDir).toAbsolutePath().normalize();
            safeCreateDir(dir);

            // Generate unique final filename
            String base = stripExtension(safeName);
            String finalName = base + "_" + Instant.now().toEpochMilli() + "_" + randomHex(8) + "." + ext;
            Path finalPath = dir.resolve(finalName).normalize();

            // Ensure the final path is within the destination directory
            if (!finalPath.startsWith(dir)) return null;

            // Write to temp file then move atomically if possible
            Path tmp = Files.createTempFile(dir, "upload_", ".tmp");
            try (OutputStream os = Files.newOutputStream(tmp, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                os.write(data);
                os.flush();
            }

            // Set restrictive permissions where supported (POSIX)
            setRestrictivePermissions(tmp);

            try {
                Files.move(tmp, finalPath, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException ex) {
                Files.move(tmp, finalPath, StandardCopyOption.REPLACE_EXISTING);
            }

            setRestrictivePermissions(finalPath);
            return finalPath.toString();
        } catch (Exception e) {
            return null;
        }
    }

    // Utility: sanitize filename to safe characters and limit length
    private static String sanitizeFilename(String name) {
        // Remove any directories
        int slash = Math.max(name.lastIndexOf('/'), name.lastIndexOf('\\'));
        if (slash >= 0) name = name.substring(slash + 1);

        // Replace invalid chars
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }

        String sanitized = sb.toString();
        if (sanitized.isEmpty()) sanitized = "file";

        // Prevent hidden dot-only names and trim length
        if (sanitized.equals(".") || sanitized.equals("..")) sanitized = "file";
        if (sanitized.length() > 100) sanitized = sanitized.substring(0, 100);

        return sanitized;
    }

    private static String getExtension(String name) {
        int idx = name.lastIndexOf('.');
        if (idx < 0 || idx == name.length() - 1) return null;
        return name.substring(idx + 1).toLowerCase(Locale.ROOT);
    }

    private static String stripExtension(String name) {
        int idx = name.lastIndexOf('.');
        return (idx < 0) ? name : name.substring(0, idx);
    }

    private static String randomHex(int bytes) {
        byte[] b = new byte[bytes];
        RNG.nextBytes(b);
        StringBuilder sb = new StringBuilder(bytes * 2);
        for (byte x : b) sb.append(String.format("%02x", x));
        return sb.toString();
    }

    private static void safeCreateDir(Path dir) throws IOException {
        Files.createDirectories(dir);
        setRestrictivePermissions(dir);
    }

    private static void setRestrictivePermissions(Path path) {
        try {
            // Only attempt on POSIX file systems
            Set<PosixFilePermission> perms = new HashSet<>();
            if (Files.isDirectory(path)) {
                // drwxr-x---
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                perms.add(PosixFilePermission.OWNER_EXECUTE);
                perms.add(PosixFilePermission.GROUP_READ);
                perms.add(PosixFilePermission.GROUP_EXECUTE);
            } else {
                // -rw-r-----
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                perms.add(PosixFilePermission.GROUP_READ);
            }
            Files.setPosixFilePermissions(path, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
            // Non-POSIX or unable to set; ignore
        }
    }

    // Magic-byte/content checks
    private static boolean contentMatchesExtension(byte[] data, String ext) {
        switch (ext) {
            case "png":
                return startsWith(data, new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A});
            case "jpg":
            case "jpeg":
                return startsWith(data, new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF});
            case "pdf":
                return startsWith(data, "%PDF-".getBytes(StandardCharsets.US_ASCII));
            case "txt":
                return isLikelyText(data);
            default:
                return false;
        }
    }

    private static boolean startsWith(byte[] data, byte[] prefix) {
        if (data.length < prefix.length) return false;
        for (int i = 0; i < prefix.length; i++) {
            if (data[i] != prefix[i]) return false;
        }
        return true;
    }

    private static boolean isLikelyText(byte[] data) {
        if (data.length == 0) return true;
        int printable = 0;
        for (byte b : data) {
            int u = b & 0xFF;
            if (u == 9 || u == 10 || u == 13 || (u >= 32 && u <= 126)) printable++;
            else if (u >= 128) printable++; // allow utf-8 bytes loosely
        }
        return ((double) printable / data.length) > 0.95;
    }

    // Test harness with 5 cases
    public static void main(String[] args) {
        String dir = "uploads_java";
        // 1) Valid text upload
        String r1 = uploadFile("greeting.txt", "Hello, secure world!\n".getBytes(StandardCharsets.UTF_8), dir);
        System.out.println("Case1: " + r1);

        // 2) Valid PNG header
        byte[] png = new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00};
        String r2 = uploadFile("image.png", png, dir);
        System.out.println("Case2: " + r2);

        // 3) Attempt traversal and weird chars
        String r3 = uploadFile("../evil..//name?.txt", "safe text".getBytes(StandardCharsets.UTF_8), dir);
        System.out.println("Case3: " + r3);

        // 4) Disallowed extension
        String r4 = uploadFile("run.exe", new byte[]{0x4D, 0x5A}, dir);
        System.out.println("Case4: " + r4);

        // 5) Too large file
        byte[] big = new byte[(int)Math.min(Integer.MAX_VALUE, MAX_SIZE + 1)];
        String r5 = uploadFile("big.txt", big, dir);
        System.out.println("Case5: " + r5);
    }
}