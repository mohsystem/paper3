import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.channels.SeekableByteChannel;
import java.util.EnumSet;

public class Lse48 {
    private static final Path BASE_DIR = Paths.get("static").toAbsolutePath().normalize();
    private static final long MAX_SIZE = 5L * 1024L * 1024L; // 5 MiB

    // Validate filename: only allow [A-Za-z0-9._-], length 1..128, no slashes
    private static String sanitizeName(String name) {
        if (name == null) throw new IllegalArgumentException("invalid filename");
        if (name.length() < 1 || name.length() > 128) throw new IllegalArgumentException("invalid filename");
        for (int i = 0; i < name.length(); i++) {
            char c = name.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '-';
            if (!ok) throw new IllegalArgumentException("invalid filename");
        }
        return name;
    }

    // Equivalent to: called when visiting /img?file=<filename>
    // Accepts input as parameter and returns the file bytes.
    public static byte[] img(String fileParam) throws IOException {
        String filename = sanitizeName(fileParam);
        Path target = BASE_DIR.resolve(filename).normalize();

        // Open without following symlinks
        try (SeekableByteChannel ch = Files.newByteChannel(
                target,
                EnumSet.of(StandardOpenOption.READ),
                LinkOption.NOFOLLOW_LINKS)) {

            long size = ch.size();
            if (size < 0 || size > MAX_SIZE) {
                throw new IOException("file too large or invalid");
            }
            if (size > Integer.MAX_VALUE) {
                throw new IOException("file too large for buffer");
            }
            ByteBuffer buf = ByteBuffer.allocate((int) size);
            while (buf.hasRemaining()) {
                int r = ch.read(buf);
                if (r < 0) break;
            }
            buf.flip();
            byte[] out = new byte[buf.remaining()];
            buf.get(out);
            return out;
        }
    }

    // Helpers for tests
    private static void writeFile(String name, byte[] data) throws IOException {
        Path p = BASE_DIR.resolve(name);
        Files.write(p, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    private static void writeBig(String name, long size) throws IOException {
        Path p = BASE_DIR.resolve(name);
        try (SeekableByteChannel ch = Files.newByteChannel(
                p,
                EnumSet.of(StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE))) {
            ch.position(size - 1);
            ch.write(ByteBuffer.wrap(new byte[]{0}));
        }
    }

    public static void main(String[] args) {
        try {
            Files.createDirectories(BASE_DIR);
            writeFile("hello.txt", "Hello, world!".getBytes(StandardCharsets.UTF_8));
            writeFile("empty.bin", new byte[0]);
            writeBig("big.bin", MAX_SIZE + 1024); // too big

            // Attempt to create a symlink pointing outside (may require privileges)
            try {
                Path link = BASE_DIR.resolve("link.txt");
                if (Files.exists(link)) Files.delete(link);
                Path targetOutside = Paths.get("/etc/hosts");
                Files.createSymbolicLink(link, targetOutside);
            } catch (Exception ignored) {
                // Symlink creation not permitted; test will handle gracefully.
            }

            // 5 test cases:
            // 1) valid small file
            try {
                byte[] b = img("hello.txt");
                System.out.println("Test1 hello.txt size=" + b.length + " ok");
            } catch (Exception e) {
                System.out.println("Test1 failed: " + e.getMessage());
            }

            // 2) empty file
            try {
                byte[] b = img("empty.bin");
                System.out.println("Test2 empty.bin size=" + b.length + " ok");
            } catch (Exception e) {
                System.out.println("Test2 failed: " + e.getMessage());
            }

            // 3) traversal attempt (invalid characters)
            try {
                byte[] b = img("../etc/passwd");
                System.out.println("Test3 traversal size=" + b.length);
            } catch (Exception e) {
                System.out.println("Test3 expected failure: " + e.getMessage());
            }

            // 4) symlink attempt
            try {
                byte[] b = img("link.txt");
                System.out.println("Test4 symlink size=" + b.length);
            } catch (Exception e) {
                System.out.println("Test4 expected failure or skipped: " + e.getMessage());
            }

            // 5) oversized file
            try {
                byte[] b = img("big.bin");
                System.out.println("Test5 big.bin size=" + b.length);
            } catch (Exception e) {
                System.out.println("Test5 expected failure: " + e.getMessage());
            }
        } catch (Exception e) {
            System.out.println("Setup error: " + e.getMessage());
        }
    }
}