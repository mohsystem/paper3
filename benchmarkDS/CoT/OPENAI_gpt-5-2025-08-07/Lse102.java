import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

public class Lse102 {
    // Chain-of-Through Step 1: Problem understanding & function signature
    // Accepts a file name and content, opens/creates with rw-r--r--, writes content, returns bytes written.
    public static long secureOpenWrite(String fileName, String content) throws IOException {
        if (fileName == null || content == null) {
            throw new IllegalArgumentException("fileName/content cannot be null");
        }
        Path path = Paths.get(fileName);

        // Chain-of-Through Step 2: Security requirements
        // - Avoid following symlinks.
        // - Use safe file creation when not exists.
        // - Set precise permissions to 0644.
        // - Ensure data is flushed.

        boolean existsNoFollow = Files.exists(path, LinkOption.NOFOLLOW_LINKS);
        if (existsNoFollow && Files.isSymbolicLink(path)) {
            throw new IOException("Refusing to open symbolic link: " + fileName);
        }

        // Chain-of-Through Step 3: Secure coding generation
        OpenOption[] options;
        if (!existsNoFollow) {
            options = new OpenOption[] {
                StandardOpenOption.READ,
                StandardOpenOption.WRITE,
                StandardOpenOption.CREATE_NEW
            };
        } else {
            options = new OpenOption[] {
                StandardOpenOption.READ,
                StandardOpenOption.WRITE,
                StandardOpenOption.TRUNCATE_EXISTING
            };
        }

        try (FileChannel fc = FileChannel.open(path, options)) {
            // Attempt to set POSIX permissions to 0644 (rw-r--r--)
            try {
                PosixFileAttributeView view = Files.getFileAttributeView(path, PosixFileAttributeView.class, LinkOption.NOFOLLOW_LINKS);
                if (view != null) {
                    Set<PosixFilePermission> perms = new HashSet<>();
                    perms.add(PosixFilePermission.OWNER_READ);
                    perms.add(PosixFilePermission.OWNER_WRITE);
                    perms.add(PosixFilePermission.GROUP_READ);
                    perms.add(PosixFilePermission.OTHERS_READ);
                    Files.setPosixFilePermissions(path, perms);
                } else {
                    // Fallback on non-POSIX (best effort)
                    File f = path.toFile();
                    // Clear permissive flags (best-effort) then set specific
                    f.setReadable(false, false);
                    f.setWritable(false, false);
                    f.setExecutable(false, false);
                    f.setReadable(true, true);   // owner read
                    f.setWritable(true, true);   // owner write
                    f.setReadable(true, false);  // everyone read
                }
            } catch (UnsupportedOperationException ignored) {
                // Ignore if POSIX permissions not supported
            }

            byte[] data = content.getBytes(StandardCharsets.UTF_8);
            ByteBuffer buf = ByteBuffer.wrap(data);
            long total = 0;
            while (buf.hasRemaining()) {
                int n = fc.write(buf);
                if (n < 0) break;
                total += n;
            }
            fc.force(true);
            return total;
        } catch (FileAlreadyExistsException e) {
            // Race condition safeguard: if create_new failed due to unexpected existence
            // try again with open/truncate if it's not a symlink
            if (Files.isSymbolicLink(path)) {
                throw new IOException("Refusing to open symbolic link after race: " + fileName, e);
            }
            try (FileChannel fc = FileChannel.open(path,
                    StandardOpenOption.READ, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                byte[] data = content.getBytes(StandardCharsets.UTF_8);
                ByteBuffer buf = ByteBuffer.wrap(data);
                long total = 0;
                while (buf.hasRemaining()) {
                    int n = fc.write(buf);
                    if (n < 0) break;
                    total += n;
                }
                fc.force(true);
                return total;
            }
        }
    }

    // Chain-of-Through Step 4 & 5 implicitly through careful code and handling above.

    public static void main(String[] args) {
        String content = "important_config";
        String[] files = new String[] {
            "important_config",
            "important_config_1",
            "important_config_2",
            "important_config",     // re-open existing
            "important_config_readwrite"
        };

        for (int i = 0; i < files.length; i++) {
            try {
                long written = secureOpenWrite(files[i], content);
                System.out.println("Test " + (i+1) + ": Wrote " + written + " bytes to " + files[i]);
            } catch (Exception e) {
                System.out.println("Test " + (i+1) + " failed for " + files[i] + ": " + e.getMessage());
            }
        }
    }
}