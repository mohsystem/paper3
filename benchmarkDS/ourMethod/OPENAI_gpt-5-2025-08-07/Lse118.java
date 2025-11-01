import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.Objects;

public class Lse118 {

    public static String generateAndWrite(Path baseDir, String filename) throws IOException {
        Objects.requireNonNull(baseDir, "baseDir");
        Objects.requireNonNull(filename, "filename");

        // Validate base directory (must be a real directory, not a symlink)
        BasicFileAttributes baseAttrs = Files.readAttributes(baseDir, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!baseAttrs.isDirectory() || baseAttrs.isSymbolicLink()) {
            throw new IOException("Invalid base directory");
        }

        // Validate filename
        if (!isValidFilename(filename)) {
            throw new IOException("Invalid filename");
        }

        // Generate 3 random floats and concatenate
        SecureRandom sr = new SecureRandom();
        float f1 = sr.nextFloat();
        float f2 = sr.nextFloat();
        float f3 = sr.nextFloat();
        String content = Float.toString(f1) + Float.toString(f2) + Float.toString(f3);

        // Prepare target path and temp file in the same directory
        Path target = baseDir.resolve(filename).normalize();
        if (!target.getParent().equals(baseDir.normalize())) {
            throw new IOException("Path traversal detected");
        }

        FileAttribute<?>[] attrs;
        try {
            attrs = new FileAttribute<?>[]{PosixFilePermissions.asFileAttribute(PosixFilePermissions.fromString("rw-------"))};
        } catch (UnsupportedOperationException e) {
            attrs = new FileAttribute<?>[]{};
        }

        Path tmp = Files.createTempFile(baseDir, "tmp-", ".tmp", attrs);

        // Write with fsync
        try (FileChannel ch = FileChannel.open(tmp, StandardOpenOption.WRITE)) {
            ByteBuffer buf = StandardCharsets.UTF_8.encode(content);
            while (buf.hasRemaining()) {
                ch.write(buf);
            }
            ch.force(true);
        } catch (IOException ioe) {
            try {
                Files.deleteIfExists(tmp);
            } catch (IOException ignore) {
                // ignore
            }
            throw ioe;
        }

        // Atomic move (fail if target exists)
        try {
            Files.move(tmp, target, StandardCopyOption.ATOMIC_MOVE);
        } catch (AtomicMoveNotSupportedException e) {
            // Fallback to non-atomic move but still safe within same filesystem
            Files.move(tmp, target);
        } finally {
            Files.deleteIfExists(tmp);
        }

        // Best-effort directory sync
        try (FileChannel dirCh = FileChannel.open(baseDir, StandardOpenOption.READ)) {
            dirCh.force(true);
        } catch (IOException ignore) {
            // ignore
        }

        return content;
    }

    private static boolean isValidFilename(String name) {
        if (name == null || name.length() == 0 || name.length() > 255) return false;
        if (name.equals(".") || name.equals("..")) return false;
        if (name.contains("/") || name.contains("\\") ) return false;
        return name.matches("^[A-Za-z0-9._-]+$");
    }

    public static void main(String[] args) {
        try {
            Path baseDir = Files.createTempDirectory("safe-out-");
            System.out.println("Base dir: " + baseDir.toAbsolutePath());

            String r1 = generateAndWrite(baseDir, "case1.txt");
            System.out.println("case1 -> " + r1);

            String r2 = generateAndWrite(baseDir, "case2.txt");
            System.out.println("case2 -> " + r2);

            String r3 = generateAndWrite(baseDir, "case3.txt");
            System.out.println("case3 -> " + r3);

            String r4 = generateAndWrite(baseDir, "case4.txt");
            System.out.println("case4 -> " + r4);

            String r5 = generateAndWrite(baseDir, "case5.txt");
            System.out.println("case5 -> " + r5);
        } catch (IOException e) {
            System.err.println("Error");
        }
    }
}