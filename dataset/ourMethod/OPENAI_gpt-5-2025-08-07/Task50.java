import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

public class Task50 {

    // Max upload size to demonstrate input validation (1 MiB).
    private static final int MAX_SIZE = 1 * 1024 * 1024;
    private static final SecureRandom RNG = new SecureRandom();

    public static String uploadFile(String baseDirStr, String fileName, byte[] data) {
        try {
            // Validate inputs
            if (baseDirStr == null || baseDirStr.trim().isEmpty()) {
                return "ERROR: Base directory is required.";
            }
            if (fileName == null) {
                return "ERROR: File name is required.";
            }
            if (data == null) {
                return "ERROR: Data is required.";
            }
            if (data.length == 0) {
                return "ERROR: No data provided.";
            }
            if (data.length > MAX_SIZE) {
                return "ERROR: File too large.";
            }
            if (!fileName.matches("^[A-Za-z0-9._-]{1,128}$")) {
                return "ERROR: Invalid file name.";
            }
            if (fileName.equals(".") || fileName.equals("..")) {
                return "ERROR: Invalid file name.";
            }

            Path base = Paths.get(baseDirStr).toAbsolutePath().normalize();
            // Create base directory if needed using secure defaults
            Files.createDirectories(base);
            // Ensure base is not a symlink
            if (Files.isSymbolicLink(base)) {
                return "ERROR: Base directory cannot be a symlink.";
            }

            Path baseReal;
            try {
                baseReal = base.toRealPath(LinkOption.NOFOLLOW_LINKS);
            } catch (IOException e) {
                return "ERROR: Unable to resolve base directory.";
            }

            Path target = baseReal.resolve(fileName).normalize();
            // Ensure target is exactly inside base
            if (!target.getParent().equals(baseReal)) {
                return "ERROR: Invalid path resolution.";
            }

            // Reject if an existing target is a symlink
            if (Files.exists(target, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(target)) {
                return "ERROR: Target path is an unsafe symlink.";
            }

            // Prepare temp file in same directory for atomic move
            String tmpSuffix = randomHex(16);
            Path temp = baseReal.resolve(fileName + ".tmp." + tmpSuffix);

            // Ensure temp does not exist
            if (Files.exists(temp, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Temporary file collision.";
            }

            // Set restrictive permissions on temp file if POSIX is supported
            FileAttribute<Set<PosixFilePermission>> attrs = null;
            try {
                if (Files.getFileStore(baseReal).supportsFileAttributeView("posix")) {
                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                    attrs = PosixFilePermissions.asFileAttribute(perms);
                }
            } catch (Exception ignored) {
                // Non-POSIX or attribute view not supported; continue without attrs
            }

            // Create temp file atomically
            if (attrs != null) {
                Files.createFile(temp, attrs);
            } else {
                Files.createFile(temp);
            }

            // Write using FileChannel and force to disk
            try (FileChannel ch = FileChannel.open(temp, StandardOpenOption.WRITE)) {
                int offset = 0;
                int chunk = 8192;
                while (offset < data.length) {
                    int len = Math.min(chunk, data.length - offset);
                    ch.write(ByteBuffer.wrap(data, offset, len));
                    offset += len;
                }
                ch.force(true);
            }

            // Move atomically to final destination if supported. Fail if dest exists.
            try {
                Files.move(temp, target, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                // If atomic move not supported, try a regular move but only if destination doesn't exist.
                if (Files.exists(target, LinkOption.NOFOLLOW_LINKS)) {
                    // Cleanup temp
                    safeDelete(temp);
                    return "ERROR: File already exists.";
                }
                Files.move(temp, target);
            }

            // Verify final file is a regular file (no symlinks)
            if (!Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) {
                safeDelete(target);
                return "ERROR: Target is not a regular file.";
            }

            // Set restrictive permissions on final file if possible (POSIX)
            try {
                if (Files.getFileStore(target).supportsFileAttributeView("posix")) {
                    Set<PosixFilePermission> perms = new HashSet<>();
                    perms.add(PosixFilePermission.OWNER_READ);
                    perms.add(PosixFilePermission.OWNER_WRITE);
                    Files.setPosixFilePermissions(target, perms);
                }
            } catch (Exception ignored) {
                // Ignore on non-POSIX systems
            }

            return "OK: Stored file '" + fileName + "' (" + data.length + " bytes).";
        } catch (Exception ex) {
            return "ERROR: " + sanitizeError(ex.getMessage());
        }
    }

    private static void safeDelete(Path p) {
        try {
            if (p != null) Files.deleteIfExists(p);
        } catch (Exception ignored) {
        }
    }

    private static String randomHex(int bytesLen) {
        byte[] b = new byte[bytesLen];
        RNG.nextBytes(b);
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (byte x : b) {
            sb.append(String.format("%02x", x));
        }
        return sb.toString();
    }

    private static String sanitizeError(String msg) {
        if (msg == null) return "Operation failed.";
        msg = msg.replaceAll("[\\r\\n]+", " ").trim();
        if (msg.length() > 200) msg = msg.substring(0, 200);
        return msg;
    }

    // Demo tests
    public static void main(String[] args) throws IOException {
        Path base = Files.createTempDirectory("task50_java_base");
        String baseDir = base.toString();

        // Test 1: Valid upload
        System.out.println(uploadFile(baseDir, "file1.txt", "Hello, world!".getBytes(StandardCharsets.UTF_8)));

        // Test 2: Another valid upload
        byte[] data2 = "Some binary \u0000 content".getBytes(StandardCharsets.UTF_8);
        System.out.println(uploadFile(baseDir, "safe_name.bin", data2));

        // Test 3: Invalid filename (path traversal attempt)
        System.out.println(uploadFile(baseDir, "bad/../name.txt", "x".getBytes(StandardCharsets.UTF_8)));

        // Test 4: Duplicate upload should fail (file exists)
        System.out.println(uploadFile(baseDir, "file1.txt", "New content".getBytes(StandardCharsets.UTF_8)));

        // Test 5: Oversize upload
        byte[] big = new byte[MAX_SIZE + 1];
        for (int i = 0; i < big.length; i++) big[i] = (byte) (i & 0xFF);
        System.out.println(uploadFile(baseDir, "big.bin", big));
    }
}