import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.EnumSet;
import java.util.Set;

public class Lse102 {
    public static String secureOpenAndWrite(String filename, String content) throws IOException {
        Path p = Paths.get(filename);

        // Reject symbolic links
        if (Files.exists(p, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(p)) {
            throw new IOException("Refusing to operate on a symbolic link: " + filename);
        }

        // Create file with 0644 if it does not exist
        if (!Files.exists(p, LinkOption.NOFOLLOW_LINKS)) {
            try {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-r--r--");
                FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                Files.createFile(p, attr);
            } catch (UnsupportedOperationException e) {
                // Filesystem may not support POSIX perms (e.g., Windows)
                Files.createFile(p);
            }
        }

        // If POSIX perms are supported, enforce 0644
        try {
            FileStore store = Files.getFileStore(p);
            if (store.supportsFileAttributeView("posix")) {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-r--r--");
                Files.setPosixFilePermissions(p, perms);
            }
        } catch (UnsupportedOperationException ignored) {
        }

        // Open for read/write and truncate to write exactly the provided content
        try (SeekableByteChannel ch = Files.newByteChannel(
                p,
                EnumSet.of(StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING))) {

            byte[] data = content.getBytes(StandardCharsets.UTF_8);
            ByteBuffer buf = ByteBuffer.wrap(data);
            while (buf.hasRemaining()) {
                ch.write(buf);
            }

            ch.position(0);

            ByteArrayOutputStream out = new ByteArrayOutputStream();
            ByteBuffer rbuf = ByteBuffer.allocate(4096);
            int n;
            while ((n = ch.read(rbuf)) > 0) {
                rbuf.flip();
                out.write(rbuf.array(), 0, n);
                rbuf.clear();
            }
            return out.toString(StandardCharsets.UTF_8);
        }
    }

    public static void main(String[] args) {
        String filename = "important_config";
        String content = "important_config";
        for (int i = 0; i < 5; i++) {
            try {
                String result = secureOpenAndWrite(filename, content);
                System.out.println("Test " + (i + 1) + ": " + result);
            } catch (IOException e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
    }
}