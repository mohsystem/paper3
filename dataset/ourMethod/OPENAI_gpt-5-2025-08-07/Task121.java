import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Objects;
import java.util.regex.Pattern;

public final class Task121 {
    private static final int MAX_SIZE = 1_048_576; // 1 MiB max
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,100}$");

    public static final class UploadResult {
        public final boolean success;
        public final String message;
        public final String savedPath;

        public UploadResult(boolean success, String message, String savedPath) {
            this.success = success;
            this.message = message;
            this.savedPath = savedPath;
        }

        @Override
        public String toString() {
            return "UploadResult{success=" + success + ", message='" + message + "', savedPath='" + savedPath + "'}";
        }
    }

    public static UploadResult uploadFile(String baseDirPath, String originalName, byte[] data) {
        if (baseDirPath == null || baseDirPath.isEmpty()) {
            return new UploadResult(false, "Base directory is required.", null);
        }
        if (originalName == null || originalName.isEmpty()) {
            return new UploadResult(false, "Filename is required.", null);
        }
        if (!NAME_PATTERN.matcher(originalName).matches()) {
            return new UploadResult(false, "Invalid filename. Allowed: A-Z, a-z, 0-9, ., _, - with length 1..100.", null);
        }
        if (data == null) {
            return new UploadResult(false, "Data must not be null.", null);
        }
        if (data.length == 0) {
            return new UploadResult(false, "File is empty.", null);
        }
        if (data.length > MAX_SIZE) {
            return new UploadResult(false, "File too large. Max " + MAX_SIZE + " bytes.", null);
        }

        Path temp = null;
        try {
            Path baseDir = Paths.get(baseDirPath);
            Files.createDirectories(baseDir);
            Path baseReal = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);

            // Create temp file in base directory
            temp = Files.createTempFile(baseReal, "upload_", ".tmp");
            // Restrict permissions where supported
            try {
                PosixFileAttributeView view = Files.getFileAttributeView(temp, PosixFileAttributeView.class, LinkOption.NOFOLLOW_LINKS);
                if (view != null) {
                    view.setPermissions(EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE));
                }
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX FS; skip setting POSIX permissions
            }

            try (FileChannel ch = FileChannel.open(temp, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                ByteBuffer buf = ByteBuffer.wrap(data);
                while (buf.hasRemaining()) {
                    ch.write(buf);
                }
                ch.force(true);
            }

            String baseName;
            String ext;
            int dot = originalName.lastIndexOf('.');
            if (dot > 0 && dot < originalName.length() - 1) {
                baseName = originalName.substring(0, dot);
                ext = originalName.substring(dot);
            } else {
                baseName = originalName;
                ext = "";
            }

            Path finalPath = null;
            for (int counter = 0; counter < 10000; counter++) {
                String candidateName = (counter == 0) ? (baseName + ext) : (baseName + "(" + counter + ")" + ext);
                Path candidate = baseReal.resolve(candidateName).normalize();
                if (!Objects.equals(candidate.getParent(), baseReal)) {
                    // Should not happen due to regex, but guard anyway
                    continue;
                }
                try {
                    // Atomic move prevents TOCTOU when target didn't exist
                    Files.move(temp, candidate, StandardCopyOption.ATOMIC_MOVE);
                    finalPath = candidate;
                    temp = null; // moved; no need to delete
                    break;
                } catch (FileAlreadyExistsException e) {
                    // Try next suffix
                } catch (AtomicMoveNotSupportedException e) {
                    try {
                        Files.move(temp, candidate);
                        finalPath = candidate;
                        temp = null;
                        break;
                    } catch (FileAlreadyExistsException ex) {
                        // Try next suffix
                    }
                }
            }

            if (finalPath == null) {
                return new UploadResult(false, "Could not allocate a unique filename.", null);
            }

            if (!Files.isRegularFile(finalPath, LinkOption.NOFOLLOW_LINKS)) {
                try {
                    Files.deleteIfExists(finalPath);
                } catch (Exception ignored) {}
                return new UploadResult(false, "Upload failed: not a regular file.", null);
            }

            return new UploadResult(true, "Upload successful.", finalPath.toString());
        } catch (IOException ioe) {
            return new UploadResult(false, "I/O error: " + ioe.getMessage(), null);
        } finally {
            if (temp != null) {
                try {
                    Files.deleteIfExists(temp);
                } catch (IOException ignored) {}
            }
        }
    }

    public static void main(String[] args) {
        String baseDir = "uploads_java";
        byte[] data1 = "Hello, world!".getBytes(StandardCharsets.UTF_8);
        byte[] data2 = "Another file content".getBytes(StandardCharsets.UTF_8);
        byte[] data3 = new byte[MAX_SIZE + 1]; // too large
        byte[] data4 = "Small".getBytes(StandardCharsets.UTF_8);
        byte[] data5 = "Duplicate name test".getBytes(StandardCharsets.UTF_8);

        UploadResult r1 = uploadFile(baseDir, "hello.txt", data1);
        System.out.println("Test 1: " + r1);

        UploadResult r2 = uploadFile(baseDir, "bad/evil.txt", data2);
        System.out.println("Test 2: " + r2);

        UploadResult r3 = uploadFile(baseDir, "big.bin", data3);
        System.out.println("Test 3: " + r3);

        UploadResult r4 = uploadFile(baseDir, "../secret.txt", data4);
        System.out.println("Test 4: " + r4);

        UploadResult r5a = uploadFile(baseDir, "hello.txt", data5);
        System.out.println("Test 5a: " + r5a);
        UploadResult r5b = uploadFile(baseDir, "hello.txt", data5);
        System.out.println("Test 5b: " + r5b);
    }
}