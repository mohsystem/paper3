import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.HashSet;
import java.util.Set;

public class Lse105 {

    public static boolean createSecretFile(String filename, String content) {
        if (filename == null || filename.isEmpty()) return false;
        try {
            Path path = Paths.get(filename).normalize();
            if (path.isAbsolute() || path.getNameCount() != 1) {
                return false; // disallow path traversal or directories
            }

            // Ensure parent directory exists (current dir) and path not a directory
            if (Files.exists(path, LinkOption.NOFOLLOW_LINKS)) {
                // Fail if exists to avoid overwriting
                return false;
            }

            boolean posix = Files.getFileAttributeView(path.getParent() == null ? Paths.get(".") : path.getParent(), PosixFileAttributeView.class) != null;

            if (posix) {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                FileAttribute<Set<PosixFilePermission>> attr = PosixFilePermissions.asFileAttribute(perms);
                Files.createFile(path, attr);
            } else {
                Files.createFile(path);
            }

            try (SeekableByteChannel ch = Files.newByteChannel(path, StandardOpenOption.WRITE)) {
                byte[] bytes = content.getBytes(StandardCharsets.UTF_8);
                ByteBuffer buf = ByteBuffer.wrap(bytes);
                while (buf.hasRemaining()) {
                    ch.write(buf);
                }
            }

            // Set restrictive permissions after write as well
            if (posix) {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
            } else {
                File f = path.toFile();
                // Owner-only read/write, no execute
                f.setReadable(false, false);
                f.setWritable(false, false);
                f.setExecutable(false, false);
                f.setReadable(true, true);
                f.setWritable(true, true);
                f.setExecutable(false, true);
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        String secret = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        System.out.println(createSecretFile("secret.txt", secret));
        System.out.println(createSecretFile("secret1.txt", secret));
        System.out.println(createSecretFile("secret2.txt", secret));
        System.out.println(createSecretFile("secret3.txt", secret));
        System.out.println(createSecretFile("secret4.txt", secret));
    }
}