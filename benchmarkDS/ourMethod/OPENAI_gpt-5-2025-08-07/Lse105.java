import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.time.Instant;
import java.util.EnumSet;
import java.util.Objects;
import java.util.Set;

public class Lse105 {

    /**
     * Create a file with the given filename inside baseDir and write the provided content.
     * The file is written safely via a temporary file then atomically moved into place.
     * The resulting file permissions are set to be restrictive (rw------- where supported).
     *
     * @param baseDir base directory (must be an existing non-symlink directory)
     * @param filename target filename (no path separators or traversal)
     * @param content content to write
     * @return the Path to the created file
     * @throws IOException on failure
     */
    public static Path createSecretFile(Path baseDir, String filename, String content) throws IOException {
        Objects.requireNonNull(baseDir, "baseDir");
        Objects.requireNonNull(filename, "filename");
        Objects.requireNonNull(content, "content");

        // Validate baseDir
        if (!Files.exists(baseDir, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Base directory not found");
        }
        if (Files.isSymbolicLink(baseDir)) {
            throw new IOException("Base directory must not be a symlink");
        }
        BasicFileAttributes attrs = Files.readAttributes(baseDir, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isDirectory()) {
            throw new IOException("Base path is not a directory");
        }

        // Validate filename
        if (filename.length() == 0 || filename.length() > 255) {
            throw new IOException("Invalid filename length");
        }
        if (filename.equals(".") || filename.equals("..")) {
            throw new IOException("Invalid filename");
        }
        if (filename.contains("/") || filename.contains("\\")) {
            throw new IOException("Invalid filename characters");
        }
        if (!filename.matches("^[A-Za-z0-9._-]+$")) {
            throw new IOException("Filename contains disallowed characters");
        }

        Path target = baseDir.resolve(filename).normalize();
        if (!target.getParent().equals(baseDir.normalize())) {
            throw new IOException("Resolved outside base directory");
        }

        // Create a restrictive attribute set if POSIX is supported
        FileAttribute<Set<PosixFilePermission>> posix0600 = null;
        boolean posixSupported = false;
        try {
            baseDir.getFileSystem().provider().readAttributes(baseDir, "posix:permissions", LinkOption.NOFOLLOW_LINKS);
            posixSupported = true;
        } catch (Exception ignored) {
            posixSupported = false;
        }
        if (posixSupported) {
            posix0600 = PosixFilePermissions.asFileAttribute(EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE));
        }

        // Create temp file securely in baseDir
        Path tmp;
        if (posixSupported) {
            tmp = Files.createTempFile(baseDir, ".tmp.", ".tmp", posix0600);
        } else {
            tmp = Files.createTempFile(baseDir, ".tmp.", ".tmp");
        }

        // Write content and fsync
        byte[] data = content.getBytes(StandardCharsets.UTF_8);
        try (FileOutputStream fos = new FileOutputStream(tmp.toFile(), false);
             FileChannel fc = fos.getChannel()) {
            fos.write(data);
            fos.flush();
            fc.force(true);
        }

        // Atomically move into place (fails if target exists)
        Files.move(tmp, target, StandardCopyOption.ATOMIC_MOVE);

        // Enforce restrictive permissions
        if (posixSupported) {
            try {
                Files.setPosixFilePermissions(target, EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE));
            } catch (UnsupportedOperationException ignored) {
                // Fallback below
            }
        }
        // Cross-platform fallback (best-effort on non-POSIX)
        File f = target.toFile();
        // Make sure others cannot read/write/execute
        f.setReadable(false, false);
        f.setWritable(false, false);
        f.setExecutable(false, false);
        // Restrict to owner read/write
        f.setReadable(true, true);
        f.setWritable(true, true);
        f.setExecutable(false, true);

        return target;
    }

    public static void main(String[] args) {
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        // Create 5 isolated test directories and write secret.txt in each
        for (int i = 1; i <= 5; i++) {
            try {
                Path dir = Files.createTempDirectory("secure_secret_test_" + i + "_" + Instant.now().toEpochMilli() + "_");
                Path p = createSecretFile(dir, "secret.txt", content);
                // Minimal verification: exists and size matches
                long size = Files.size(p);
                if (size != content.getBytes(StandardCharsets.UTF_8).length) {
                    System.out.println("Test " + i + " FAILED: size mismatch.");
                } else {
                    System.out.println("Test " + i + " OK: " + p.toString());
                }
            } catch (Exception e) {
                System.out.println("Test " + i + " FAILED");
            }
        }
    }
}