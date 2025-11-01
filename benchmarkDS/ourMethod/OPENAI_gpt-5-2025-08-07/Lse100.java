import java.io.File;
import java.io.IOException;
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

public class Lse100 {

    // Returns: 0 created, 1 already exists, negative values indicate errors
    public static int createImportantConfig(String baseDir) {
        if (baseDir == null || baseDir.isEmpty()) {
            return -1;
        }

        final String filename = "important_config";
        Path baseRaw = Paths.get(baseDir);

        // Reject if base is a symlink (best-effort)
        try {
            if (Files.isSymbolicLink(baseRaw)) {
                return -2;
            }
        } catch (Exception e) {
            return -3;
        }

        Path baseResolved;
        try {
            // Resolve to real path without following final symlink of base itself
            baseResolved = baseRaw.toRealPath(LinkOption.NOFOLLOW_LINKS);
        } catch (IOException e) {
            return -4;
        }

        try {
            if (!Files.isDirectory(baseResolved, LinkOption.NOFOLLOW_LINKS)) {
                return -5;
            }
        } catch (Exception e) {
            return -6;
        }

        Path target = baseResolved.resolve(filename);

        // Attempt to create with secure POSIX permissions when available
        boolean posix = false;
        try {
            posix = FileSystems.getDefault().supportedFileAttributeViews().contains("posix");
        } catch (Exception ignored) {
            posix = false;
        }

        if (posix) {
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
            try (SeekableByteChannel ch = Files.newByteChannel(
                    target,
                    EnumSet.of(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE),
                    attr)) {
                // Validate attributes without following links
                BasicFileAttributes bfa = Files.readAttributes(target, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
                if (!bfa.isRegularFile()) {
                    return -7;
                }
                // Force metadata to storage
                ch.force(true);
                return 0;
            } catch (FileAlreadyExistsException e) {
                return 1;
            } catch (IOException e) {
                return -8;
            }
        } else {
            // Non-POSIX fallback: create exclusively, then restrict best-effort
            try {
                // CREATE_NEW to avoid races
                try (SeekableByteChannel ch = Files.newByteChannel(
                        target,
                        EnumSet.of(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE))) {
                    BasicFileAttributes bfa = Files.readAttributes(target, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
                    if (!bfa.isRegularFile()) {
                        return -9;
                    }
                    ch.force(true);
                }
                // Best-effort restrictive perms on non-POSIX filesystems
                File f = target.toFile();
                // Remove all permissions, then add owner read/write
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(true, true);
                f.setWritable(true, true);
                return 0;
            } catch (FileAlreadyExistsException e) {
                return 1;
            } catch (IOException e) {
                return -10;
            }
        }
    }

    private static void printResult(int r) {
        System.out.println("Result: " + r);
    }

    public static void main(String[] args) {
        try {
            // Create a temporary base directory for testing
            Path base = Files.createTempDirectory("secure_base_");
            // Test case 1: create in base directory
            printResult(createImportantConfig(base.toString()));

            // Test case 2: create again in the same directory (should report exists)
            printResult(createImportantConfig(base.toString()));

            // Test case 3: nested directory
            Path nested = base.resolve("nestedA");
            Files.createDirectories(nested);
            printResult(createImportantConfig(nested.toString()));

            // Test case 4: another nested directory
            Path nested2 = base.resolve("nestedB");
            Files.createDirectories(nested2);
            printResult(createImportantConfig(nested2.toString()));

            // Test case 5: symlink base (if supported), expect failure or handle gracefully
            Path linkBase = base.resolve("link_to_nestedA");
            try {
                Files.deleteIfExists(linkBase);
                Files.createSymbolicLink(linkBase, nested);
                printResult(createImportantConfig(linkBase.toString()));
            } catch (UnsupportedOperationException | IOException | SecurityException e) {
                // If symlinks unsupported, fallback to valid dir to ensure 5th test runs
                printResult(createImportantConfig(base.toString()));
            }
        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
        }
    }
}