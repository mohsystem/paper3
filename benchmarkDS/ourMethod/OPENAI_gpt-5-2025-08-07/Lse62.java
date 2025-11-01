import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Arrays;
import java.util.UUID;

public class Lse62 {

    public static class Result {
        public final long bytesRead;
        public final long checksum;
        public final boolean empty;

        public Result(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum & 0xFFFFFFFFL;
            this.empty = (this.checksum == 0L);
        }

        @Override
        public String toString() {
            return "Result{bytesRead=" + bytesRead + ", checksum=" + checksum + ", empty=" + empty + "}";
        }
    }

    private static final long MAX_FILE_SIZE = 16L * 1024L * 1024L; // 16 MiB

    private static boolean isSafeRelative(String rel) {
        if (rel == null || rel.isEmpty()) return false;
        Path p = Paths.get(rel);
        if (p.isAbsolute()) return false;
        for (Path part : p) {
            String s = part.toString();
            if ("..".equals(s) || s.contains("\0")) return false;
        }
        return true;
    }

    private static Path resolveUnderBase(Path base, String rel) throws IOException {
        Path p = base.resolve(rel).normalize();
        if (!p.startsWith(base)) {
            throw new IOException("Path escapes base directory");
        }
        return p;
    }

    public static Result processFile(String baseDir, String inputRel, String outputRel) throws IOException {
        if (!isSafeRelative(inputRel) || !isSafeRelative(outputRel)) {
            throw new IOException("Unsafe relative path");
        }

        Path base = Paths.get(baseDir).toAbsolutePath().normalize();
        Files.createDirectories(base);

        Path inPath = resolveUnderBase(base, inputRel);
        // Prevent symlink traversal on final component
        if (Files.isSymbolicLink(inPath)) {
            throw new IOException("Input is a symlink");
        }
        // Validate regular file and reasonable size
        BasicFileAttributes attrs = Files.readAttributes(inPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isRegularFile()) {
            throw new IOException("Input is not a regular file");
        }
        long size = attrs.size();
        if (size < 0 || size > MAX_FILE_SIZE) {
            throw new IOException("File size invalid or exceeds limit");
        }

        long bytesRead = 0;
        long checksum = 0;

        // Read entire file into buffer and compute checksum
        byte[] buf = new byte[(int) size];
        try (FileChannel fc = FileChannel.open(inPath, StandardOpenOption.READ)) {
            ByteBuffer bb = ByteBuffer.wrap(buf);
            while (bb.hasRemaining()) {
                int n = fc.read(bb);
                if (n < 0) break;
                bytesRead += n;
            }
        }

        for (int i = 0; i < bytesRead; i++) {
            checksum = (checksum + (buf[i] & 0xFF)) & 0xFFFFFFFFL;
        }

        // Save results atomically to output file
        Path outPath = resolveUnderBase(base, outputRel);
        Path parent = outPath.getParent();
        if (parent == null) parent = base;
        Files.createDirectories(parent);

        String data = Long.toString(bytesRead) + " " + Long.toString(checksum) + System.lineSeparator();
        byte[] outBytes = data.getBytes(StandardCharsets.UTF_8);

        String tmpName = outPath.getFileName().toString() + ".tmp-" + UUID.randomUUID().toString();
        Path tmpPath = parent.resolve(tmpName);
        try (FileChannel out = FileChannel.open(tmpPath, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            ByteBuffer outBuf = ByteBuffer.wrap(outBytes);
            while (outBuf.hasRemaining()) {
                out.write(outBuf);
            }
            out.force(true);
        }
        // Atomic move into place
        try {
            Files.move(tmpPath, outPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tmpPath, outPath, StandardCopyOption.REPLACE_EXISTING);
        }

        return new Result(bytesRead, checksum);
    }

    private static void safeWriteInput(Path base, String rel, byte[] data) throws IOException {
        if (!isSafeRelative(rel)) throw new IOException("Unsafe rel path");
        Path p = resolveUnderBase(base, rel);
        Path parent = p.getParent();
        if (parent != null) Files.createDirectories(parent);

        Path tmp = parent.resolve(p.getFileName().toString() + ".tmp-" + UUID.randomUUID());
        try (FileChannel out = FileChannel.open(tmp, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            ByteBuffer bb = ByteBuffer.wrap(data);
            while (bb.hasRemaining()) out.write(bb);
            out.force(true);
        }
        try {
            Files.move(tmp, p, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tmp, p, StandardCopyOption.REPLACE_EXISTING);
        }
    }

    public static void main(String[] args) {
        try {
            Path base = Paths.get("sandbox_java").toAbsolutePath().normalize();
            Files.createDirectories(base);

            // Prepare 5 input files
            safeWriteInput(base, "inputs/empty.bin", new byte[0]);
            safeWriteInput(base, "inputs/hello.txt", "hello".getBytes(StandardCharsets.UTF_8));
            byte[] kb = new byte[1024];
            Arrays.fill(kb, (byte) 'A');
            safeWriteInput(base, "inputs/1kb.bin", kb);
            byte[] seq = new byte[256];
            for (int i = 0; i < 256; i++) seq[i] = (byte) i;
            safeWriteInput(base, "inputs/seq.bin", seq);
            byte[] many = new byte[10000];
            Arrays.fill(many, (byte) '0');
            safeWriteInput(base, "inputs/zeros.txt", many);

            // 5 test cases
            Result r1 = processFile(base.toString(), "inputs/empty.bin", "outputs/empty.out");
            System.out.println("T1: " + r1);
            Result r2 = processFile(base.toString(), "inputs/hello.txt", "outputs/hello.out");
            System.out.println("T2: " + r2);
            Result r3 = processFile(base.toString(), "inputs/1kb.bin", "outputs/1kb.out");
            System.out.println("T3: " + r3);
            Result r4 = processFile(base.toString(), "inputs/seq.bin", "outputs/seq.out");
            System.out.println("T4: " + r4);
            Result r5 = processFile(base.toString(), "inputs/zeros.txt", "outputs/zeros.out");
            System.out.println("T5: " + r5);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}