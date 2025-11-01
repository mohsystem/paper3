import java.io.IOException;
import java.io.OutputStream;
import java.lang.management.ManagementFactory;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse72 {

    private static final long MAX_SIZE = 10L * 1024 * 1024;
    private static final Pattern SAFE_NAME = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");

    public static String uploadFile(String filename, byte[] data) throws IOException {
        if (filename == null || data == null) {
            throw new IOException("Invalid input");
        }
        if (!SAFE_NAME.matcher(filename).matches()) {
            throw new IOException("Invalid filename");
        }
        if (data.length > MAX_SIZE) {
            throw new IOException("File too large");
        }

        Path base = Paths.get("").toAbsolutePath().normalize();
        Path uploads = base.resolve("uploads").normalize();

        // Create uploads directory if absent and ensure it is a real directory (no symlink)
        Files.createDirectories(uploads);
        if (Files.isSymbolicLink(uploads) || !Files.isDirectory(uploads, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Invalid uploads directory");
        }

        // Create a unique temp file name
        String pidPart = ManagementFactory.getRuntimeMXBean().getName().split("@")[0];
        String tmpName = ".tmp-" + pidPart + "-" + Long.toHexString(Double.doubleToLongBits(Math.random()));
        Path tmpPath = uploads.resolve(tmpName).normalize();

        // Ensure tmpPath is within uploads
        if (!tmpPath.getParent().equals(uploads)) {
            throw new IOException("Invalid temp path");
        }

        // Create new temp file (exclusive)
        try (OutputStream os = Files.newOutputStream(tmpPath, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            // Set restrictive permissions if supported
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(tmpPath, perms);
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX FS; ignore
            }

            os.write(data);
            os.flush();
        } catch (IOException e) {
            // Cleanup on failure to write
            try { Files.deleteIfExists(tmpPath); } catch (IOException ignored) {}
            throw e;
        }

        // Validate regular file
        BasicFileAttributes attrs = Files.readAttributes(tmpPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isRegularFile()) {
            try { Files.deleteIfExists(tmpPath); } catch (IOException ignored) {}
            throw new IOException("Not a regular file");
        }

        // Destination path
        Path dest = uploads.resolve(filename).normalize();
        if (!dest.getParent().equals(uploads)) {
            try { Files.deleteIfExists(tmpPath); } catch (IOException ignored) {}
            throw new IOException("Invalid destination path");
        }

        // Create a hard link for atomic no-overwrite placement
        try {
            Files.createLink(dest, tmpPath); // fails if dest exists
        } catch (IOException e) {
            try { Files.deleteIfExists(tmpPath); } catch (IOException ignored) {}
            throw new IOException("Save failed: " + e.getMessage(), e);
        }

        // Remove temp file name
        try { Files.deleteIfExists(tmpPath); } catch (IOException ignored) {}

        return dest.toString();
    }

    private static void runTest(String name, byte[] data) {
        try {
            String path = uploadFile(name, data);
            System.out.println("OK: " + name + " -> " + path);
        } catch (Exception e) {
            System.out.println("ERR: " + name + " -> " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest("a.txt", "hello".getBytes(StandardCharsets.UTF_8));                // success
        runTest("b.png", new byte[] {0x01, 0x02, 0x03});                           // success
        runTest("../bad.txt", "oops".getBytes(StandardCharsets.UTF_8));            // traversal -> error
        runTest("bad/inner.txt", "oops".getBytes(StandardCharsets.UTF_8));         // separator -> error
        runTest("big.bin", new byte[(int) (MAX_SIZE + 1)]);                        // too large -> error
    }
}