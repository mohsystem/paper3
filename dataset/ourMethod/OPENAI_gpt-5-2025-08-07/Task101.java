import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;
import java.util.UUID;

public class Task101 {

    public static boolean createExecutableScript(String baseDirStr, String content) {
        if (baseDirStr == null || content == null) {
            return false;
        }
        if (baseDirStr.length() == 0 || baseDirStr.length() > 4096) {
            return false;
        }

        final Path baseDir = Paths.get(baseDirStr).toAbsolutePath().normalize();

        try {
            // Create base directory if it doesn't exist, then validate it's a directory and not a symlink
            if (!Files.exists(baseDir, LinkOption.NOFOLLOW_LINKS)) {
                Files.createDirectories(baseDir);
            }
            if (Files.isSymbolicLink(baseDir)) {
                return false;
            }
            if (!Files.isDirectory(baseDir, LinkOption.NOFOLLOW_LINKS)) {
                return false;
            }

            // Tighten permissions on baseDir where possible (POSIX)
            try {
                Set<PosixFilePermission> permsDir = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(baseDir, permsDir);
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX file system, skip
            }

            Path realBase = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
            Path finalPath = realBase.resolve("script.sh").normalize();

            // Ensure finalPath is within base directory
            if (!finalPath.getParent().equals(realBase)) {
                return false;
            }

            // Fail if target exists already (no overwrite)
            if (Files.exists(finalPath, LinkOption.NOFOLLOW_LINKS)) {
                return false; // refuse to overwrite, including if it's a symlink
            }

            // Create a temp file in the same directory for atomic move
            String tmpName = ".script.sh.tmp-" + UUID.randomUUID();
            Path tempPath = realBase.resolve(tmpName);

            // Ensure we don't follow symlinks when creating temp file; CREATE_NEW to prevent races
            try (FileChannel ch = FileChannel.open(
                    tempPath,
                    StandardOpenOption.CREATE_NEW,
                    StandardOpenOption.WRITE
            )) {
                byte[] data = content.getBytes(StandardCharsets.UTF_8);
                ByteBuffer buf = ByteBuffer.wrap(data);
                while (buf.hasRemaining()) {
                    ch.write(buf);
                }
                ch.force(true);
            } catch (FileAlreadyExistsException e) {
                // Retry once with another name
                tempPath = realBase.resolve(".script.sh.tmp-" + UUID.randomUUID());
                try (FileChannel ch = FileChannel.open(
                        tempPath,
                        StandardOpenOption.CREATE_NEW,
                        StandardOpenOption.WRITE
                )) {
                    byte[] data = content.getBytes(StandardCharsets.UTF_8);
                    ByteBuffer buf = ByteBuffer.wrap(data);
                    while (buf.hasRemaining()) {
                        ch.write(buf);
                    }
                    ch.force(true);
                }
            }

            // Set secure permissions on temp file (owner rwx)
            try {
                Set<PosixFilePermission> permsFile = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(tempPath, permsFile);
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX: best-effort
                tempPath.toFile().setReadable(true, true);
                tempPath.toFile().setWritable(true, true);
                tempPath.toFile().setExecutable(true, true);
            }

            // Atomically move temp to final without overwrite
            boolean moved = false;
            try {
                Files.move(tempPath, finalPath, StandardCopyOption.ATOMIC_MOVE);
                moved = true;
            } catch (AtomicMoveNotSupportedException e) {
                // Fallback to non-atomic move within same directory (still atomic on most file systems)
                Files.move(tempPath, finalPath);
                moved = true;
            } catch (FileAlreadyExistsException e) {
                // Target appeared meanwhile; do not overwrite
                moved = false;
            } finally {
                if (!moved) {
                    try { Files.deleteIfExists(tempPath); } catch (IOException ignored) {}
                }
            }

            if (!moved) {
                return false;
            }

            // Ensure final is executable
            try {
                Set<PosixFilePermission> permsFinal = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(finalPath, permsFinal);
            } catch (UnsupportedOperationException ignored) {
                finalPath.toFile().setExecutable(true, true);
            }

            return true;
        } catch (IOException e) {
            return false;
        }
    }

    private static String tmpBase(String suffix) {
        String base = System.getProperty("java.io.tmpdir");
        return Paths.get(base, "Task101_" + suffix).toString();
    }

    public static void main(String[] args) {
        String content = "#!/bin/sh\necho \"Hello from script\"\n";

        // Test 1: Valid base directory
        String t1 = tmpBase("test1");
        System.out.println("Test1: " + createExecutableScript(t1, content));

        // Test 2: Non-existing nested directory (should be created)
        String t2 = Paths.get(tmpBase("test2"), "nested").toString();
        System.out.println("Test2: " + createExecutableScript(t2, content));

        // Test 3: Path with traversal that normalizes within base
        String base3a = Paths.get(tmpBase("test3"), "a").toString();
        String t3 = Paths.get(base3a, "..", "b").toString();
        System.out.println("Test3: " + createExecutableScript(t3, content));

        // Prepare Test 4: Pre-create script.sh so creation should fail
        String t4 = tmpBase("test4");
        try {
            Path dir4 = Paths.get(t4);
            Files.createDirectories(dir4);
            Path pre = dir4.resolve("script.sh");
            if (!Files.exists(pre, LinkOption.NOFOLLOW_LINKS)) {
                Files.write(pre, "#!/bin/sh\necho preexisting\n".getBytes(StandardCharsets.UTF_8),
                        StandardOpenOption.CREATE_NEW);
            }
        } catch (IOException ignored) {}
        System.out.println("Test4 (exists): " + createExecutableScript(t4, content));

        // Test 5: Base path is a file (should fail)
        String t5file = tmpBase("test5_as_file");
        try {
            Files.write(Paths.get(t5file), "not a dir".getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        } catch (IOException ignored) {}
        System.out.println("Test5 (base is file): " + createExecutableScript(t5file, content));
    }
}