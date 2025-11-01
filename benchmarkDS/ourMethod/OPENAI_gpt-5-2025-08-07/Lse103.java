import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.EnumSet;

public class Lse103 {

    private static final int MAX_NAME_LENGTH = 255;

    private static void requireSimpleFilename(String filename) throws IOException {
        if (filename == null || filename.isEmpty()) {
            throw new IOException("invalid name");
        }
        if (filename.length() > MAX_NAME_LENGTH) {
            throw new IOException("name too long");
        }
        if (filename.equals(".") || filename.equals("..")) {
            throw new IOException("invalid name");
        }
        // Disallow any separators to force a single filename (no subdirs, no traversal)
        if (filename.contains("/") || filename.contains("\\") || filename.contains("\0")) {
            throw new IOException("invalid name");
        }
    }

    private static Path resolveAndValidateBase(String baseDir) throws IOException {
        if (baseDir == null || baseDir.isEmpty()) {
            throw new IOException("invalid base dir");
        }
        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.exists(base, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("base not found");
        }
        if (!Files.isDirectory(base, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("base not dir");
        }
        if (Files.isSymbolicLink(base)) {
            throw new IOException("base is symlink");
        }
        return base;
    }

    // CREATE_NEW -> like O_CREAT | O_EXCL | O_WRONLY
    public static int secureCreateNewFile(String baseDir, String filename, byte[] content) throws IOException {
        requireSimpleFilename(filename);
        Path base = resolveAndValidateBase(baseDir);
        Path target = base.resolve(filename).normalize();

        if (!target.getParent().equals(base)) {
            throw new IOException("path escape");
        }

        EnumSet<StandardOpenOption> opts = EnumSet.of(
                StandardOpenOption.CREATE_NEW,
                StandardOpenOption.WRITE
        );

        int written;
        try (FileChannel fc = FileChannel.open(target, opts)) {
            ByteBuffer buf = ByteBuffer.wrap(content == null ? new byte[0] : content);
            written = 0;
            while (buf.hasRemaining()) {
                int n = fc.write(buf);
                if (n < 0) throw new IOException("write error");
                written += n;
            }
            fc.force(true);
        }
        // Best-effort fsync directory for durability
        try (FileChannel dirFc = FileChannel.open(base, StandardOpenOption.READ)) {
            dirFc.force(true);
        } catch (IOException ignored) {
            // Some platforms do not allow opening directories with FileChannel
        }
        return written;
    }

    // CREATE + TRUNCATE_EXISTING -> like O_CREAT | O_TRUNC | O_WRONLY
    public static int secureTruncateAndWriteFile(String baseDir, String filename, byte[] content) throws IOException {
        requireSimpleFilename(filename);
        Path base = resolveAndValidateBase(baseDir);
        Path target = base.resolve(filename).normalize();

        if (!target.getParent().equals(base)) {
            throw new IOException("path escape");
        }

        EnumSet<StandardOpenOption> opts = EnumSet.of(
                StandardOpenOption.CREATE,
                StandardOpenOption.TRUNCATE_EXISTING,
                StandardOpenOption.WRITE
        );

        int written;
        try (FileChannel fc = FileChannel.open(target, opts)) {
            ByteBuffer buf = ByteBuffer.wrap(content == null ? new byte[0] : content);
            written = 0;
            while (buf.hasRemaining()) {
                int n = fc.write(buf);
                if (n < 0) throw new IOException("write error");
                written += n;
            }
            fc.force(true);
        }
        // Best-effort fsync directory
        try (FileChannel dirFc = FileChannel.open(base, StandardOpenOption.READ)) {
            dirFc.force(true);
        } catch (IOException ignored) {
        }
        return written;
    }

    public static void main(String[] args) {
        try {
            Path base = Files.createTempDirectory("secure_open_demo_");
            String baseDir = base.toString();

            // 1) Create new file (success)
            int n1 = secureCreateNewFile(baseDir, "a.txt", "hello".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test1 create_new a.txt bytes=" + n1);

            // 2) Create same file again (should fail)
            try {
                secureCreateNewFile(baseDir, "a.txt", "x".getBytes(StandardCharsets.UTF_8));
                System.out.println("Test2 unexpected success");
            } catch (IOException e) {
                System.out.println("Test2 expected failure");
            }

            // 3) Truncate and write existing file
            int n3 = secureTruncateAndWriteFile(baseDir, "a.txt", "world".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test3 truncate a.txt bytes=" + n3);

            // 4) Create new different file
            int n4 = secureCreateNewFile(baseDir, "b.txt", "data".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test4 create_new b.txt bytes=" + n4);

            // 5) Invalid path traversal attempt
            try {
                secureCreateNewFile(baseDir, "../evil.txt", "bad".getBytes(StandardCharsets.UTF_8));
                System.out.println("Test5 unexpected success");
            } catch (IOException e) {
                System.out.println("Test5 expected failure");
            }
        } catch (IOException e) {
            System.out.println("fatal");
        }
    }
}