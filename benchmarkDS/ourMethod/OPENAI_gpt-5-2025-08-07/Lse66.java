import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

public class Lse66 {

    private static final long MAX_PDF_SIZE = 10L * 1024L * 1024L; // 10 MiB

    public static Path savePdf(byte[] data, String originalFilename, Path baseUploadsDir) throws IOException {
        if (data == null || originalFilename == null || baseUploadsDir == null) {
            throw new IOException("invalid input");
        }
        if (data.length < 5 || data.length > MAX_PDF_SIZE) {
            throw new IOException("invalid size");
        }
        // Validate PDF header
        byte[] hdr = "%PDF-".getBytes(StandardCharsets.US_ASCII);
        for (int i = 0; i < hdr.length; i++) {
            if (data[i] != hdr[i]) {
                throw new IOException("invalid pdf header");
            }
        }

        // Validate base directory is an existing directory and not a symlink
        if (!Files.exists(baseUploadsDir, LinkOption.NOFOLLOW_LINKS) ||
            !Files.isDirectory(baseUploadsDir, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("base directory invalid");
        }

        // Ensure base path is absolute and normalized
        baseUploadsDir = baseUploadsDir.toAbsolutePath().normalize();

        // Sanitize filename
        String name = sanitizeFilename(originalFilename);
        if (!name.toLowerCase().endsWith(".pdf")) {
            throw new IOException("invalid extension");
        }

        // Resolve destination path securely and ensure it remains within base dir
        Path dest = baseUploadsDir.resolve(name).normalize();
        if (!dest.startsWith(baseUploadsDir)) {
            throw new IOException("path traversal");
        }

        // Create a temp file in the uploads directory with restrictive permissions
        Path tmp = null;
        try {
            // Generate a random suffix to avoid collisions
            SecureRandom rnd = new SecureRandom();
            String suffix = Long.toHexString(rnd.nextLong()) + Long.toHexString(rnd.nextLong());
            String prefix = "upload_";
            try {
                tmp = Files.createTempFile(baseUploadsDir, prefix, "_" + suffix + ".tmp");
            } catch (UnsupportedOperationException e) {
                // Fallback to fixed name if TempFile not supported
                tmp = baseUploadsDir.resolve(prefix + suffix + ".tmp");
                Files.createFile(tmp);
            }

            // Set restrictive permissions where supported (POSIX)
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(tmp, perms);
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX FS; ignore
            }

            // Open the temp file channel, write, and fsync
            try (FileChannel fc = FileChannel.open(tmp, StandardOpenOption.WRITE)) {
                ByteBuffer buf = ByteBuffer.wrap(data);
                long written = 0;
                while (buf.hasRemaining()) {
                    int w = fc.write(buf);
                    if (w < 0) throw new IOException("write failed");
                    written += w;
                    if (written > MAX_PDF_SIZE) {
                        throw new IOException("size exceeded");
                    }
                }
                fc.force(true);
            }

            // Optionally fsync the directory to persist directory entry (best-effort)
            try (FileChannel dirc = FileChannel.open(baseUploadsDir, StandardOpenOption.READ)) {
                dirc.force(true);
            } catch (Exception ignored) {
                // Some platforms don't allow syncing directories; ignore
            }

            // Atomically move temp into place; do not overwrite existing files
            Files.move(tmp, dest, StandardCopyOption.ATOMIC_MOVE);
            return dest;
        } catch (AtomicMoveNotSupportedException e) {
            // Fallback to non-atomic move if ATOMIC_MOVE not supported, but still respect non-overwrite
            if (tmp != null) {
                try {
                    Path created = Files.move(tmp, dest);
                    return created;
                } catch (FileAlreadyExistsException faee) {
                    throw new IOException("destination exists");
                }
            }
            throw new IOException("move failed");
        } finally {
            // Cleanup temp if it still exists
            if (tmp != null) {
                try {
                    Files.deleteIfExists(tmp);
                } catch (Exception ignored) {
                }
            }
        }
    }

    private static String sanitizeFilename(String name) {
        // Extract base name (drop any path components)
        int slash = Math.max(name.lastIndexOf('/'), name.lastIndexOf('\\'));
        if (slash >= 0) {
            name = name.substring(slash + 1);
        }
        // Allow only safe characters, replace others with underscore
        StringBuilder sb = new StringBuilder();
        String lower = name.toLowerCase();
        for (int i = 0; i < lower.length() && i < 128; i++) {
            char c = lower.charAt(i);
            if ((c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        if (sb.length() == 0) {
            sb.append("upload.pdf");
        }
        // Ensure it ends with .pdf
        String out = sb.toString();
        if (!out.endsWith(".pdf")) {
            // strip trailing dots to avoid hidden files
            while (out.endsWith(".")) out = out.substring(0, out.length() - 1);
            out = out + ".pdf";
        }
        // Collapse multiple dots
        out = out.replace("..", ".");
        // Avoid leading dot (hidden files)
        if (out.startsWith(".")) out = "u" + out;
        return out;
    }

    public static void main(String[] args) {
        try {
            Path uploads = Paths.get("uploads");
            if (!Files.exists(uploads, LinkOption.NOFOLLOW_LINKS)) {
                Files.createDirectories(uploads);
            }

            // Test 1: Valid PDF
            byte[] pdf1 = ("%PDF-1.7\n%...\n").getBytes(StandardCharsets.US_ASCII);
            try {
                Path p1 = savePdf(pdf1, "report.pdf", uploads);
                System.out.println("OK1: " + p1.toString());
            } catch (Exception e) {
                System.out.println("FAIL1");
            }

            // Test 2: Path traversal attempt
            try {
                Path p2 = savePdf(pdf1, "../../evil.pdf", uploads);
                System.out.println("OK2: " + p2.toString());
            } catch (Exception e) {
                System.out.println("FAIL2");
            }

            // Test 3: Wrong extension
            try {
                Path p3 = savePdf(pdf1, "image.png", uploads);
                System.out.println("OK3: " + p3.toString());
            } catch (Exception e) {
                System.out.println("FAIL3");
            }

            // Test 4: Exceed size limit
            byte[] big = new byte[(int) (MAX_PDF_SIZE + 1)];
            byte[] header = "%PDF-".getBytes(StandardCharsets.US_ASCII);
            System.arraycopy(header, 0, big, 0, header.length);
            try {
                Path p4 = savePdf(big, "big.pdf", uploads);
                System.out.println("OK4: " + p4.toString());
            } catch (Exception e) {
                System.out.println("FAIL4");
            }

            // Test 5: Odd characters in filename
            try {
                Path p5 = savePdf(pdf1, "a..//b??.pdf", uploads);
                System.out.println("OK5: " + p5.toString());
            } catch (Exception e) {
                System.out.println("FAIL5");
            }

        } catch (Exception e) {
            System.out.println("setup failed");
        }
    }
}