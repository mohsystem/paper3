import java.io.IOException;
import java.io.OutputStream;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;
import java.util.UUID;

public class Task101 {

    // Creates script.sh in the given directory with the provided content, makes it executable.
    // Returns the absolute path to script.sh on success, or null on failure.
    public static String createExecutableScript(String dirPath, String content, boolean overwrite) {
        String defaultContent = "#!/bin/sh\necho \"Hello from script.sh\"\n";
        String data = (content == null || content.isEmpty()) ? defaultContent : content;
        // Ensure trailing newline for scripts
        if (!data.endsWith("\n")) {
            data = data + "\n";
        }

        try {
            Path dir = Paths.get((dirPath == null || dirPath.isBlank()) ? "." : dirPath).toAbsolutePath().normalize();
            Files.createDirectories(dir);

            Path finalPath = dir.resolve("script.sh").normalize();
            // Avoid path traversal resolving outside dir
            if (!finalPath.getParent().equals(dir)) {
                return null;
            }

            if (!overwrite && Files.exists(finalPath, LinkOption.NOFOLLOW_LINKS)) {
                return null;
            }

            Path tmpPath = dir.resolve("script.sh.tmp-" + UUID.randomUUID()).normalize();
            // Create temp file exclusively
            try (FileChannel ch = FileChannel.open(tmpPath,
                    StandardOpenOption.CREATE_NEW,
                    StandardOpenOption.WRITE)) {

                byte[] bytes = data.getBytes(StandardCharsets.UTF_8);
                ch.write(java.nio.ByteBuffer.wrap(bytes));
                ch.force(true);
            }

            // Set restrictive permissions on temp file (best-effort; ignore if not supported)
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(tmpPath, perms);
            } catch (UnsupportedOperationException ignored) {
                tmpPath.toFile().setReadable(true, true);
                tmpPath.toFile().setWritable(true, true);
                tmpPath.toFile().setExecutable(true, true);
            }

            // Move into place atomically
            CopyOption[] moveOpts = overwrite
                    ? new CopyOption[]{StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING}
                    : new CopyOption[]{StandardCopyOption.ATOMIC_MOVE};
            try {
                Files.move(tmpPath, finalPath, moveOpts);
            } catch (AtomicMoveNotSupportedException e) {
                // Fallback without ATOMIC_MOVE
                if (overwrite) {
                    Files.move(tmpPath, finalPath, StandardCopyOption.REPLACE_EXISTING);
                } else {
                    Files.move(tmpPath, finalPath);
                }
            }

            // Ensure executable permissions on final file (best-effort)
            try {
                Set<PosixFilePermission> perms = EnumSet.of(
                        PosixFilePermission.OWNER_READ,
                        PosixFilePermission.OWNER_WRITE,
                        PosixFilePermission.OWNER_EXECUTE
                );
                Files.setPosixFilePermissions(finalPath, perms);
            } catch (UnsupportedOperationException ignored) {
                finalPath.toFile().setReadable(true, true);
                finalPath.toFile().setWritable(true, true);
                finalPath.toFile().setExecutable(true, true);
            }

            // Ensure the result is a regular file, not a symlink
            if (Files.isSymbolicLink(finalPath)) {
                try {
                    Files.deleteIfExists(finalPath);
                } catch (Exception ignored) {}
                return null;
            }

            return finalPath.toString();
        } catch (IOException e) {
            return null;
        } finally {
            // Best-effort cleanup of any stray tmp file if exception occurred (ignored here)
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        try {
            Path d1 = Files.createTempDirectory("t101_java_1");
            Path d2 = Files.createTempDirectory("t101_java_2");

            String r1 = createExecutableScript(d1.toString(), "#!/bin/sh\necho Case1", false);
            System.out.println("Test1: " + r1);

            // Attempt to create again without overwrite (should fail -> null)
            String r2 = createExecutableScript(d1.toString(), "#!/bin/sh\necho Case2", false);
            System.out.println("Test2: " + r2);

            // Overwrite existing
            String r3 = createExecutableScript(d1.toString(), "#!/bin/sh\necho Case3 Overwrite", true);
            System.out.println("Test3: " + r3);

            // New directory
            String r4 = createExecutableScript(d2.toString(), "echo Case4 Content only", false);
            System.out.println("Test4: " + r4);

            // Current directory
            String r5 = createExecutableScript(".", "echo Case5 in CWD", true);
            System.out.println("Test5: " + r5);

        } catch (IOException e) {
            System.out.println("Setup failed: " + e.getMessage());
        }
    }
}