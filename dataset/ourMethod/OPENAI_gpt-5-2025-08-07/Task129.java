import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.util.Objects;
import java.util.regex.Pattern;

public class Task129 {

    private static final Pattern SAFE_FILE_NAME = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");
    private static final int MAX_SIZE_BYTES = 1024 * 1024; // 1 MiB

    public static void main(String[] args) {
        String baseDir = "server_files";
        try {
            setupTestFiles(baseDir);
        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
            return;
        }

        // 5 test cases
        String[] testFiles = new String[]{
                "sample1.txt",      // valid
                "../secret.txt",    // invalid traversal attempt
                "/etc/passwd",      // invalid absolute path
                "missing.txt",      // non-existent
                "safe-2.log"        // valid
        };

        for (String tf : testFiles) {
            String result = retrieveFileContent(baseDir, tf);
            System.out.println("Request: '" + tf + "' => " + result);
        }
    }

    // Retrieves the content of a file from baseDir with security checks.
    public static String retrieveFileContent(String baseDir, String fileName) {
        try {
            if (!isValidFileName(fileName)) {
                return "ERROR: Invalid file name format.";
            }
            Objects.requireNonNull(baseDir, "baseDir");
            Path base = Paths.get(baseDir).toAbsolutePath().normalize();

            if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Base directory does not exist.";
            }

            Path target = base.resolve(fileName).normalize();

            // Ensure path remains within base directory
            if (!target.startsWith(base)) {
                return "ERROR: Access denied.";
            }

            // Deny symlinks and non-regular files
            if (Files.isSymbolicLink(target)) {
                return "ERROR: Symlinks are not allowed.";
            }
            if (!Files.exists(target, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: File not found.";
            }
            if (!Files.isRegularFile(target, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Not a regular file.";
            }

            long size = Files.size(target);
            if (size < 0 || size > MAX_SIZE_BYTES) {
                return "ERROR: File too large.";
            }

            byte[] data;
            try (FileChannel ch = FileChannel.open(target, StandardOpenOption.READ)) {
                ByteBuffer buf = ByteBuffer.allocate((int) size);
                while (buf.hasRemaining()) {
                    int r = ch.read(buf);
                    if (r < 0) break;
                }
                data = buf.array();
            }

            return "OK: " + new String(data, StandardCharsets.UTF_8);

        } catch (SecurityException se) {
            return "ERROR: Security manager denied access.";
        } catch (IOException ioe) {
            return "ERROR: " + ioe.getMessage();
        }
    }

    private static boolean isValidFileName(String fileName) {
        if (fileName == null) return false;
        if (fileName.length() < 1 || fileName.length() > 255) return false;
        return SAFE_FILE_NAME.matcher(fileName).matches();
    }

    // Test setup: create base dir and write sample files atomically.
    private static void setupTestFiles(String baseDir) throws IOException {
        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.exists(base)) {
            Files.createDirectories(base);
        } else if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Base path exists but is not a directory");
        }

        safeWriteFileAtomic(base, "sample1.txt", "Hello from sample1.\nThis is a test file.");
        safeWriteFileAtomic(base, "safe-2.log", "Log line 1\nLog line 2\n");
    }

    private static void safeWriteFileAtomic(Path base, String fileName, String content) throws IOException {
        if (!isValidFileName(fileName)) {
            throw new IOException("Invalid file name for write: " + fileName);
        }
        Path target = base.resolve(fileName).normalize();
        if (!target.startsWith(base)) {
            throw new IOException("Target path escapes base directory");
        }

        Path temp = Files.createTempFile(base, ".tmp-", ".part");
        try {
            byte[] data = content.getBytes(StandardCharsets.UTF_8);
            if (data.length > MAX_SIZE_BYTES) {
                throw new IOException("Content too large");
            }
            Files.write(temp, data, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
            try {
                Files.move(temp, target, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(temp, target, StandardCopyOption.REPLACE_EXISTING);
            }
        } catch (IOException e) {
            try {
                Files.deleteIfExists(temp);
            } catch (IOException ignore) {
                // ignore cleanup failure
            }
            throw e;
        }
    }
}