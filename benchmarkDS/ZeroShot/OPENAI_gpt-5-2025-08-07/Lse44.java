import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.EnumSet;

public class Lse44 {

    private static final long MAX_SIZE_BYTES = 1024 * 1024; // 1MB
    private static final Path BASE_DIR;

    static {
        Path base;
        try {
            base = Paths.get("/safe").toRealPath(LinkOption.NOFOLLOW_LINKS);
        } catch (IOException e) {
            // If /safe does not exist at class load, we still set it to /safe and let later calls fail
            base = Paths.get("/safe").toAbsolutePath().normalize();
        }
        BASE_DIR = base;
    }

    public static String readSafeFile(String requested) throws IOException {
        if (requested == null || requested.isEmpty()) {
            throw new IllegalArgumentException("Invalid filename");
        }
        if (requested.indexOf('\0') >= 0) {
            throw new IllegalArgumentException("Invalid character in filename");
        }

        Path rel = Paths.get(requested);
        if (rel.isAbsolute()) {
            throw new SecurityException("Absolute paths are not allowed");
        }

        // Disallow dangerous components
        for (Path part : rel) {
            String name = part.toString();
            if (name.equals("") || name.equals(".") || name.equals("..")) {
                throw new SecurityException("Path traversal detected");
            }
        }

        Path target = BASE_DIR.resolve(rel).normalize();

        if (!target.startsWith(BASE_DIR)) {
            throw new SecurityException("Access outside of /safe is not permitted");
        }

        // Ensure no symlinks in the path from BASE_DIR to target
        Path relative = BASE_DIR.relativize(target);
        Path check = BASE_DIR;
        for (Path part : relative) {
            check = check.resolve(part);
            if (Files.isSymbolicLink(check)) {
                throw new SecurityException("Symlinks are not allowed");
            }
        }

        // Ensure it is a regular file and within size limit
        BasicFileAttributes attrs = Files.readAttributes(target, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isRegularFile()) {
            throw new SecurityException("Not a regular file");
        }
        long size = attrs.size();
        if (size > MAX_SIZE_BYTES) {
            throw new SecurityException("File too large");
        }

        try (SeekableByteChannel ch = Files.newByteChannel(target,
                EnumSet.of(StandardOpenOption.READ, StandardOpenOption.NOFOLLOW_LINKS))) {
            ByteArrayOutputStream bos = new ByteArrayOutputStream((int) Math.min(size, 8192));
            ByteBuffer buf = ByteBuffer.allocate(8192);
            long remaining = MAX_SIZE_BYTES;
            while (true) {
                buf.clear();
                int n = ch.read(buf);
                if (n < 0) break;
                if (n == 0) continue;
                remaining -= n;
                if (remaining < 0) {
                    throw new SecurityException("File grew beyond limit while reading");
                }
                bos.write(buf.array(), 0, n);
            }
            return bos.toString(StandardCharsets.UTF_8);
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "example.txt",
                "../etc/passwd",
                "/etc/passwd",
                "subdir/file.txt",
                "symlink_outside.txt"
        };
        for (String t : tests) {
            try {
                String content = readSafeFile(t);
                System.out.println("Read OK: " + t + " (len=" + content.length() + ")");
            } catch (Exception e) {
                System.out.println("Read FAIL: " + t + " -> " + e.getMessage());
            }
        }
    }
}