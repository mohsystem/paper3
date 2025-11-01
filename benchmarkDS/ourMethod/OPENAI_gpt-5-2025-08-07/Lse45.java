import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.EnumSet;

public class Lse45 {

    // Maximum file size to read (1 MiB)
    private static final long MAX_SIZE = 1L << 20;

    public static byte[] readSafeFile(String baseDirStr, String requestedRelative) throws IOException {
        if (baseDirStr == null || requestedRelative == null) {
            throw new IllegalArgumentException("Invalid input");
        }
        if (requestedRelative.length() == 0 || requestedRelative.length() > 255) {
            throw new IllegalArgumentException("Invalid path length");
        }
        if (requestedRelative.indexOf('\0') >= 0) {
            throw new IllegalArgumentException("Invalid character");
        }
        // Basic character allowlist (letters, digits, space, dot, dash, underscore, slash)
        if (!requestedRelative.matches("^[A-Za-z0-9._/ -]+$")) {
            throw new IllegalArgumentException("Invalid characters");
        }

        Path base = Paths.get(baseDirStr);
        // Resolve base directory without following symlinks
        Path baseReal = base.toRealPath(LinkOption.NOFOLLOW_LINKS);

        Path rel = Paths.get(requestedRelative);
        if (rel.isAbsolute()) {
            throw new IllegalArgumentException("Path must be relative");
        }

        // Reject any ".", ".." components
        for (Path p : rel) {
            String name = p.toString();
            if (name.equals(".") || name.equals("..") || name.isEmpty()) {
                throw new IllegalArgumentException("Invalid path components");
            }
            if (name.length() > 255) {
                throw new IllegalArgumentException("Component too long");
            }
        }

        Path candidate = baseReal.resolve(rel).normalize();
        if (!candidate.startsWith(baseReal)) {
            throw new IllegalArgumentException("Path escapes base directory");
        }

        // Open first, with NOFOLLOW_LINKS to avoid symlink traversal
        try (FileChannel ch = FileChannel.open(candidate,
                EnumSet.of(StandardOpenOption.READ),
                LinkOption.NOFOLLOW_LINKS)) {

            BasicFileAttributeView view = ch.getFileAttributeView(BasicFileAttributeView.class);
            if (view == null) {
                throw new IOException("Cannot read attributes");
            }
            BasicFileAttributes attrs = view.readAttributes();
            if (!attrs.isRegularFile()) {
                throw new IOException("Not a regular file");
            }
            long size = attrs.size();
            if (size < 0 || size > MAX_SIZE) {
                throw new IOException("File too large");
            }

            int toRead = (int) size;
            ByteBuffer buf = ByteBuffer.allocate(toRead);
            int read;
            while (buf.hasRemaining() && (read = ch.read(buf)) != -1) {
                // continue
            }
            buf.flip();
            byte[] out = new byte[buf.remaining()];
            buf.get(out);
            return out;
        }
    }

    private static void writeFile(Path p, String data) throws IOException {
        Files.createDirectories(p.getParent() == null ? p.toAbsolutePath().getParent() : p.getParent());
        Files.write(p, data.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
    }

    public static void main(String[] args) {
        try {
            Path safeDir = Files.createTempDirectory("safe_dir_").toRealPath(LinkOption.NOFOLLOW_LINKS);
            Path subDir = safeDir.resolve("sub");
            Files.createDirectories(subDir);

            // Create test files
            Path f1 = safeDir.resolve("a.txt");
            Path f2 = subDir.resolve("x.txt");
            writeFile(f1, "Hello from a.txt");
            writeFile(f2, "Nested file content");

            // Attempt to create a symlink inside the safe dir pointing outside
            Path outside = safeDir.getParent() != null ? safeDir.getParent().resolve("outside.txt") : safeDir.resolveSibling("outside.txt");
            try {
                Files.write(outside, "outside".getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            } catch (Throwable ignored) {}
            Path evilLink = safeDir.resolve("evil_link");
            try {
                Files.deleteIfExists(evilLink);
                Files.createSymbolicLink(evilLink, outside);
            } catch (UnsupportedOperationException | IOException | SecurityException ignored) {
                // Symlinks not supported or not permitted; ignore
            }

            // Test cases (5)
            String[] requests = new String[] {
                "a.txt",           // valid
                "sub/x.txt",       // valid nested
                "../etc/passwd",   // traversal attempt
                "evil_link",       // symlink attempt
                "sub"              // directory read attempt
            };

            for (String req : requests) {
                try {
                    byte[] data = readSafeFile(safeDir.toString(), req);
                    String preview = new String(data, StandardCharsets.UTF_8);
                    if (preview.length() > 80) preview = preview.substring(0, 80);
                    System.out.println("OK: " + req + " => " + preview);
                } catch (Exception e) {
                    System.out.println("FAIL: " + req);
                }
            }

        } catch (Exception e) {
            System.out.println("Initialization failed");
        }
    }
}