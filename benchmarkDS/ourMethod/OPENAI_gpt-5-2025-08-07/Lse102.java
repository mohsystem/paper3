import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

public class Lse102 {

    // Writes the string "important_config" into a file named "important_config" inside baseDir.
    // Returns true on success, false on failure.
    public static boolean writeImportantConfig(String baseDir) {
        if (baseDir == null || baseDir.isEmpty()) {
            return false;
        }
        try {
            Path base = Paths.get(baseDir).toAbsolutePath().normalize();
            if (!Files.exists(base)) {
                return false;
            }
            if (!Files.isDirectory(base)) {
                return false;
            }
            // Prevent symlink usage for base directory if possible
            try {
                if (Files.isSymbolicLink(base)) {
                    return false;
                }
            } catch (Exception ignored) {
                // If we cannot determine, proceed cautiously
            }

            Path file = base.resolve("important_config").normalize();
            if (!file.startsWith(base)) {
                return false;
            }

            boolean posix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
            Set<PosixFilePermission> perms0644 = new HashSet<>();
            if (posix) {
                perms0644.add(PosixFilePermission.OWNER_READ);
                perms0644.add(PosixFilePermission.OWNER_WRITE);
                perms0644.add(PosixFilePermission.GROUP_READ);
                perms0644.add(PosixFilePermission.OTHERS_READ);
            }

            // Create file if it doesn't exist with 0644; else open existing securely
            if (!Files.exists(file)) {
                if (posix) {
                    FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms0644);
                    Files.createFile(file, attr);
                } else {
                    Files.createFile(file);
                }
            }

            // Validate type and reject symlinks
            if (Files.isSymbolicLink(file)) {
                return false;
            }
            BasicFileAttributes attrs = Files.readAttributes(file, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                return false;
            }

            // Open for read/write, truncate to write exactly our content
            EnumSet<StandardOpenOption> opts = EnumSet.of(StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
            try (SeekableByteChannel ch = Files.newByteChannel(file, opts)) {
                byte[] data = "important_config".getBytes(StandardCharsets.UTF_8);
                ByteBuffer buf = ByteBuffer.wrap(data);
                while (buf.hasRemaining()) {
                    ch.write(buf);
                }
                if (ch instanceof FileChannel) {
                    ((FileChannel) ch).force(true);
                }
            }

            // Ensure permissions are 0644 if POSIX is available
            if (posix) {
                Files.setPosixFilePermissions(file, perms0644);
            }
            return true;
        } catch (IOException ex) {
            return false;
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        Path t1 = Files.createTempDirectory("icase1_");
        System.out.println("Test1: " + writeImportantConfig(t1.toString()));

        // Test2: write again to same dir (overwrite/truncate)
        System.out.println("Test2: " + writeImportantConfig(t1.toString()));

        // Test3: baseDir is a file (should fail)
        Path t3file = Files.createTempFile("icase3_", ".tmp");
        System.out.println("Test3: " + writeImportantConfig(t3file.toString()));

        // Test4: non-existent directory (should fail)
        Path t4 = Paths.get(t1.toString(), "nonexistent_subdir_do_not_create");
        System.out.println("Test4: " + writeImportantConfig(t4.toString()));

        // Test5: create a valid subdir and write there
        Path t5 = Files.createTempDirectory("icase5_");
        Path sub = Files.createDirectory(t5.resolve("conf"));
        System.out.println("Test5: " + writeImportantConfig(sub.toString()));
    }
}