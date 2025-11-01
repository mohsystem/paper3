import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.BasicFileAttributes;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.channels.SeekableByteChannel;
import java.security.SecureRandom;
import java.util.EnumSet;

public class Lse63 {
    private static final long MAX_SIZE = 50L * 1024L * 1024L; // 50 MB

    public static class Result {
        public final long bytesRead;
        public final long checksum; // 32-bit unsigned stored in long
        public final boolean isEmpty;

        public Result(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum & 0xFFFFFFFFL;
            this.isEmpty = (this.checksum == 0L);
        }

        @Override
        public String toString() {
            return "bytes=" + bytesRead + ", checksum=" + Long.toUnsignedString(checksum) + ", isEmpty=" + isEmpty;
        }
    }

    public static Result processFile(String inputPath, String outputPath) throws IOException {
        if (inputPath == null || outputPath == null) {
            throw new IllegalArgumentException("Paths must not be null");
        }

        Path in = Paths.get(inputPath);
        if (!Files.exists(in, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Input file does not exist: " + inputPath);
        }
        if (Files.isSymbolicLink(in)) {
            throw new IOException("Refusing to read from a symbolic link: " + inputPath);
        }
        BasicFileAttributes attrs = Files.readAttributes(in, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
        if (!attrs.isRegularFile()) {
            throw new IOException("Input is not a regular file: " + inputPath);
        }
        long size = attrs.size();
        if (size < 0 || size > MAX_SIZE) {
            throw new IOException("File size out of allowed range (0.." + MAX_SIZE + "): " + size);
        }

        byte[] buf = new byte[(int) size];
        try (SeekableByteChannel ch = Files.newByteChannel(in, EnumSet.of(StandardOpenOption.READ))) {
            ByteBuffer bb = ByteBuffer.wrap(buf);
            while (bb.hasRemaining()) {
                int r = ch.read(bb);
                if (r == -1) break;
            }
        }

        long checksum = 0L;
        for (byte b : buf) {
            checksum = (checksum + (b & 0xFF)) & 0xFFFFFFFFL;
        }
        Result result = new Result(buf.length, checksum);

        Path out = Paths.get(outputPath);
        if (out.getParent() != null) {
            Files.createDirectories(out.getParent());
        }
        if (Files.exists(out, LinkOption.NOFOLLOW_LINKS) && Files.isSymbolicLink(out)) {
            throw new IOException("Refusing to write to a symbolic link: " + outputPath);
        }
        String content = "bytes=" + result.bytesRead + System.lineSeparator()
                + "checksum=" + Long.toUnsignedString(result.checksum) + System.lineSeparator();
        try (OutputStream os = Files.newOutputStream(out, new OpenOption[]{
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE
        })) {
            os.write(content.getBytes(StandardCharsets.UTF_8));
        }
        return result;
    }

    private static void writeBytes(Path file, byte[] data) throws IOException {
        if (file.getParent() != null) {
            Files.createDirectories(file.getParent());
        }
        Files.write(file, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) {
        try {
            Path tempDir = Files.createTempDirectory("lse63_java_");
            // Test case 1: Empty file
            Path f1 = tempDir.resolve("empty.bin");
            writeBytes(f1, new byte[0]);
            Result r1 = processFile(f1.toString(), tempDir.resolve("out1.txt").toString());
            System.out.println("Test1: " + r1);

            // Test case 2: Small text
            Path f2 = tempDir.resolve("hello.txt");
            writeBytes(f2, "Hello, world!".getBytes(StandardCharsets.UTF_8));
            Result r2 = processFile(f2.toString(), tempDir.resolve("out2.txt").toString());
            System.out.println("Test2: " + r2);

            // Test case 3: 2048 random bytes
            Path f3 = tempDir.resolve("random.bin");
            byte[] rand = new byte[2048];
            new SecureRandom().nextBytes(rand);
            writeBytes(f3, rand);
            Result r3 = processFile(f3.toString(), tempDir.resolve("out3.txt").toString());
            System.out.println("Test3: " + r3);

            // Test case 4: 10000 bytes of 0xFF
            Path f4 = tempDir.resolve("ff.bin");
            byte[] ff = new byte[10000];
            for (int i = 0; i < ff.length; i++) ff[i] = (byte) 0xFF;
            writeBytes(f4, ff);
            Result r4 = processFile(f4.toString(), tempDir.resolve("out4.txt").toString());
            System.out.println("Test4: " + r4);

            // Test case 5: UTF-8 unicode text repeated
            Path f5 = tempDir.resolve("utf8.txt");
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 10; i++) sb.append("こんにちは世界");
            writeBytes(f5, sb.toString().getBytes(StandardCharsets.UTF_8));
            Result r5 = processFile(f5.toString(), tempDir.resolve("out5.txt").toString());
            System.out.println("Test5: " + r5);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}