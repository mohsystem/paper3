import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

public class Task127 {

    // Copy a file from baseDir/userRelativePath to a temporary file in tempDir.
    // Returns the absolute path of the created temp file on success, or "ERROR: <message>" on error.
    public static String copyToTemp(String baseDirStr, String userRelativePath, String tempDirStr) {
        // Validate inputs
        if (baseDirStr == null || userRelativePath == null || tempDirStr == null) {
            return "ERROR: Null input parameter";
        }
        baseDirStr = baseDirStr.trim();
        userRelativePath = userRelativePath.trim();
        tempDirStr = tempDirStr.trim();
        if (baseDirStr.isEmpty() || userRelativePath.isEmpty() || tempDirStr.isEmpty()) {
            return "ERROR: Empty input parameter";
        }
        if (userRelativePath.length() > 4096) {
            return "ERROR: Path too long";
        }
        if (userRelativePath.indexOf('\0') >= 0) {
            return "ERROR: Invalid character in path";
        }
        // Build and validate base and temp directories
        final Path baseDir = Paths.get(baseDirStr).toAbsolutePath().normalize();
        final Path tempDir = Paths.get(tempDirStr).toAbsolutePath().normalize();

        try {
            if (!Files.exists(baseDir)) {
                return "ERROR: Base directory does not exist";
            }
            if (!Files.isDirectory(baseDir, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Base path is not a directory";
            }
            if (isOrHasSymlink(baseDir, baseDir)) {
                return "ERROR: Base directory or its parents contain a symlink";
            }
            // Construct the resolved source path within base directory
            Path userPathObj = Paths.get(userRelativePath);
            if (userPathObj.isAbsolute()) {
                return "ERROR: Path must be relative to base directory";
            }
            Path resolved = baseDir.resolve(userPathObj).normalize();

            if (!resolved.startsWith(baseDir)) {
                return "ERROR: Path escapes base directory";
            }
            // Reject if any segment from base to resolved contains symlinks
            if (isOrHasSymlink(resolved, baseDir)) {
                return "ERROR: Source path contains a symlink";
            }
            if (!Files.exists(resolved, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Source file does not exist";
            }
            if (!Files.isRegularFile(resolved, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Source is not a regular file";
            }

            // Ensure temp directory exists with safe permissions
            if (!Files.exists(tempDir)) {
                Files.createDirectories(tempDir);
            }
            if (!Files.isDirectory(tempDir, LinkOption.NOFOLLOW_LINKS)) {
                return "ERROR: Temp path is not a directory";
            }
            if (isOrHasSymlink(tempDir, tempDir)) {
                return "ERROR: Temp directory or its parents contain a symlink";
            }

            // Create temp file with restrictive permissions
            Path tempFile;
            FileAttribute<?>[] attrs = new FileAttribute<?>[0];
            if (supportsPosix(tempDir)) {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE
                );
                attrs = new FileAttribute<?>[]{
                        PosixFilePermissions.asFileAttribute(perms)
                };
            }
            tempFile = Files.createTempFile(tempDir, "copy_", ".tmp", attrs);
            // For non-POSIX, try to set permissions anyway
            try {
                if (supportsPosix(tempFile)) {
                    Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE
                    );
                    Files.setPosixFilePermissions(tempFile, perms);
                }
            } catch (Exception ignore) {
                // Ignore inability to set permissions on non-POSIX systems
            }

            // Copy with channels and locks. Flush and fsync.
            try (
                    FileChannel inChannel = FileChannel.open(resolved, StandardOpenOption.READ);
                    FileLock inLock = inChannel.lock(0L, Long.MAX_VALUE, true);
                    OutputStream os = Files.newOutputStream(tempFile, StandardOpenOption.WRITE);
                    FileChannel outChannel = ((os instanceof java.io.FileOutputStream)
                            ? ((java.io.FileOutputStream) os).getChannel()
                            : FileChannel.open(tempFile, StandardOpenOption.WRITE))
            ) {
                try (InputStream is = Files.newInputStream(resolved, StandardOpenOption.READ)) {
                    byte[] buf = new byte[8192];
                    int r;
                    while ((r = is.read(buf)) != -1) {
                        os.write(buf, 0, r);
                    }
                    os.flush();
                }
                outChannel.force(true);
            } catch (IOException e) {
                safeDelete(tempFile);
                return "ERROR: I/O error during copy: " + safeMsg(e);
            }

            return tempFile.toAbsolutePath().toString();

        } catch (IOException e) {
            return "ERROR: " + safeMsg(e);
        }
    }

    private static boolean supportsPosix(Path p) {
        try {
            return Files.getFileStore(p).supportsFileAttributeView(PosixFileAttributeView.class);
        } catch (IOException e) {
            return false;
        }
    }

    private static boolean isOrHasSymlink(Path path, Path stopAtInclusive) {
        // Checks if 'path' or any of its parents down to stopAtInclusive is a symlink
        Path abs = path.toAbsolutePath().normalize();
        Path stop = stopAtInclusive.toAbsolutePath().normalize();
        for (Path p = abs; p != null; p = p.getParent()) {
            try {
                if (Files.isSymbolicLink(p)) {
                    return true;
                }
            } catch (Exception e) {
                return true;
            }
            if (p.equals(stop)) break;
        }
        return false;
    }

    private static void safeDelete(Path p) {
        try {
            if (p != null) Files.deleteIfExists(p);
        } catch (Exception ignore) {
        }
    }

    private static String safeMsg(Exception e) {
        String m = e.getMessage();
        if (m == null) return e.getClass().getSimpleName();
        return m.replaceAll("[\\r\\n\\t]", " ").trim();
    }

    // Basic demo tests per requirements
    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("base_dir_");
            Path tempOut = Files.createTempDirectory("temp_out_");
            // Create test files
            Path f1 = base.resolve("file1.txt");
            Files.write(f1, "Hello world 1".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            Path sub = base.resolve("subdir");
            Files.createDirectories(sub);
            Path f2 = sub.resolve("file2.txt");
            Files.write(f2, "Hello world 2".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);

            // Attempt to create a symlink if supported (may fail on Windows)
            Path link = base.resolve("link_to_f1");
            try {
                Files.deleteIfExists(link);
                Files.createSymbolicLink(link, f1.getFileName());
            } catch (Exception ignore) {}

            String r1 = copyToTemp(base.toString(), "file1.txt", tempOut.toString());
            System.out.println("Test1: " + r1);

            String r2 = copyToTemp(base.toString(), "subdir/file2.txt", tempOut.toString());
            System.out.println("Test2: " + r2);

            String r3 = copyToTemp(base.toString(), "../../etc/passwd", tempOut.toString());
            System.out.println("Test3: " + r3);

            String r4 = copyToTemp(base.toString(), "does_not_exist.txt", tempOut.toString());
            System.out.println("Test4: " + r4);

            String r5 = copyToTemp(base.toString(), base.resolve("file1.txt").toString(), tempOut.toString());
            System.out.println("Test5: " + r5);

        } catch (IOException e) {
            System.out.println("Setup ERROR: " + e.getMessage());
        }
    }
}