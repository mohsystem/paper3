import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;

public class Lse104 {
    public static int writeImportantConfig(String filePath, String content) throws IOException {
        if (filePath == null || filePath.isEmpty()) {
            throw new IllegalArgumentException("Invalid file path");
        }
        if (content == null) {
            throw new IllegalArgumentException("Content cannot be null");
        }

        Path path = Paths.get(filePath);

        // Reject obvious symlinks to reduce risk (non-atomic)
        if (Files.exists(path, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(path)) {
            throw new SecurityException("Refusing to open a symbolic link");
        }

        byte[] data = content.getBytes(StandardCharsets.UTF_8);
        int totalWritten = 0;

        EnumSet<StandardOpenOption> opts = EnumSet.of(StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.CREATE);
        try (FileChannel channel = FileChannel.open(path, opts)) {
            // Attempt to set owner-only permissions where supported (best-effort)
            try {
                Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
            } catch (UnsupportedOperationException ignored) {
                // Non-POSIX filesystem; ignore
            }

            ByteBuffer buffer = ByteBuffer.wrap(data);
            while (buffer.hasRemaining()) {
                int n = channel.write(buffer);
                if (n <= 0) {
                    throw new IOException("Failed to write data");
                }
                totalWritten += n;
            }
            channel.force(true);
        }

        return totalWritten;
    }

    public static void main(String[] args) {
        String[] files = {
            "java_test_secure_1.cfg",
            "java_test_secure_2.cfg",
            "java_test_secure_3.cfg",
            "java_test_secure_4.cfg",
            "java_test_secure_5.cfg"
        };
        for (String f : files) {
            try {
                int written = writeImportantConfig(f, "important_config");
                System.out.println("Java wrote " + written + " bytes to " + f);
            } catch (Exception e) {
                System.out.println("Java failed for " + f + ": " + e.getMessage());
            }
        }
    }
}