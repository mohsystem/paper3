import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.FileVisitOption;
import java.nio.file.FileVisitResult;
import java.nio.file.FileVisitResult.*;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Lse63 {

    public static final class Result {
        public final long bytesRead;
        public final long checksum;
        public final boolean success;
        public final String message;

        public Result(long bytesRead, long checksum, boolean success, String message) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
            this.success = success;
            this.message = message;
        }

        @Override
        public String toString() {
            return "Result{bytesRead=" + bytesRead + ", checksum=" + checksum + ", success=" + success + ", message='" + message + "'}";
        }
    }

    private static final long MAX_SIZE = 8L * 1024 * 1024; // 8 MiB

    private static boolean isSafeRelativePath(String rel) {
        if (rel == null || rel.isEmpty()) return false;
        if (rel.startsWith("/") || rel.startsWith("\\") || rel.contains("\0")) return false;
        String[] parts = rel.split("/");
        for (String p : parts) {
            if (p.isEmpty() || p.equals(".") || p.equals("..")) return false;
            if (p.contains("\\") || p.contains("\0")) return false;
        }
        return true;
    }

    private static void enforceWithinBase(Path baseDir, Path target) throws IOException {
        Path normBase = baseDir.toRealPath(LinkOption.NOFOLLOW_LINKS);
        Path normTargetParent = target.getParent() == null ? normBase : target.getParent();
        // Check each segment for symlink without following
        Path cur = normBase;
        Path rel = normBase.relativize(target.normalize());
        for (Path part : rel) {
            Path next = cur.resolve(part);
            if (Files.exists(next, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(next)) {
                throw new IOException("Symlink encountered in path");
            }
            cur = next;
        }
        Path normTarget = baseDir.resolve(rel).normalize();
        if (!normTarget.startsWith(normBase)) {
            throw new IOException("Path escapes base directory");
        }
    }

    private static void setOwnerOnlyPermissions(Path p) {
        try {
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(p, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
            // Non-POSIX or failure; continue without throwing
        }
    }

    public static Result process(String baseDirStr, String inputRelPath, String outputRelPath) {
        if (!isSafeRelativePath(inputRelPath) || !isSafeRelativePath(outputRelPath)) {
            return new Result(0, 0, false, "Invalid relative path");
        }
        Path baseDir = Paths.get(baseDirStr).toAbsolutePath().normalize();
        try {
            if (!Files.isDirectory(baseDir, LinkOption.NOFOLLOW_LINKS)) {
                return new Result(0, 0, false, "Base directory not found");
            }
            Path inputPath = baseDir.resolve(inputRelPath).normalize();
            enforceWithinBase(baseDir, inputPath);
            if (!Files.exists(inputPath, LinkOption.NOFOLLOW_LINKS)) {
                return new Result(0, 0, false, "Input file not found");
            }
            BasicFileAttributes attrs = Files.readAttributes(inputPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                return new Result(0, 0, false, "Input not a regular file");
            }
            if (attrs.size() > MAX_SIZE) {
                return new Result(0, 0, false, "File too large");
            }

            long bytesRead = 0;
            long checksum = 0; // 32-bit checksum modulo arithmetic
            try (BufferedInputStream bis = new BufferedInputStream(Files.newInputStream(inputPath, StandardOpenOption.READ))) {
                byte[] buf = new byte[8192];
                int n;
                while ((n = bis.read(buf)) != -1) {
                    bytesRead += n;
                    for (int i = 0; i < n; i++) {
                        checksum = (checksum + (buf[i] & 0xFF)) & 0xFFFFFFFFL;
                    }
                }
            }

            Path outputPath = baseDir.resolve(outputRelPath).normalize();
            enforceWithinBase(baseDir, outputPath);
            Path outDir = outputPath.getParent();
            if (outDir != null) {
                if (!Files.exists(outDir, LinkOption.NOFOLLOW_LINKS)) {
                    Files.createDirectories(outDir);
                }
                enforceWithinBase(baseDir, outDir);
            } else {
                outDir = baseDir;
            }

            // Create temp file in the same directory
            Path tempFile = Files.createTempFile(outDir, "tmp_", ".writing");
            setOwnerOnlyPermissions(tempFile);
            String data = bytesRead + "," + checksum + System.lineSeparator();
            byte[] outBytes = data.getBytes(StandardCharsets.UTF_8);
            try (FileChannel ch = FileChannel.open(tempFile, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
                ByteBuffer bb = ByteBuffer.wrap(outBytes);
                while (bb.hasRemaining()) {
                    ch.write(bb);
                }
                ch.force(true);
            }
            try {
                Files.move(tempFile, outputPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tempFile, outputPath, StandardCopyOption.REPLACE_EXISTING);
            }
            try {
                // best effort directory sync
                try (FileChannel dirCh = FileChannel.open(outDir, StandardOpenOption.READ)) {
                    dirCh.force(true);
                }
            } catch (IOException ignored) {}

            return new Result(bytesRead, checksum, true, "OK");
        } catch (IOException e) {
            return new Result(0, 0, false, "IO error");
        }
    }

    public static void main(String[] args) throws Exception {
        Path base = Files.createTempDirectory("secure_io_base_");
        System.out.println("Base dir: " + base);

        // Prepare test files
        Files.write(base.resolve("empty.txt"), new byte[0]);
        Files.write(base.resolve("hello.txt"), "Hello, World!".getBytes(StandardCharsets.UTF_8));
        byte[] zeros = new byte[1024];
        Files.write(base.resolve("zeros.bin"), zeros);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 1000; i++) sb.append("abc");
        Files.write(base.resolve("repeated.txt"), sb.toString().getBytes(StandardCharsets.UTF_8));
        Files.createDirectories(base.resolve("subdir"));

        // Test cases
        Result r1 = process(base.toString(), "empty.txt", "out/out1.txt");
        System.out.println("Test1: " + r1);

        Result r2 = process(base.toString(), "hello.txt", "out/out2.txt");
        System.out.println("Test2: " + r2);

        Result r3 = process(base.toString(), "zeros.bin", "out/out3.txt");
        System.out.println("Test3: " + r3);

        Result r4 = process(base.toString(), "repeated.txt", "out/out4.txt");
        System.out.println("Test4: " + r4);

        // Invalid traversal path
        Result r5 = process(base.toString(), "../evil.txt", "out/out5.txt");
        System.out.println("Test5: " + r5);
    }
}