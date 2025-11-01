import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;

public class Lse62 {

    public static final class Result {
        public final long bytesRead;
        public final long checksum; // unsigned 64-bit represented in signed long

        public Result(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }
    }

    // Processes the input file and writes "bytes_read=<num>\nchecksum=<num>\n" to output file.
    // Returns Result with the byte count and checksum (unsigned 64-bit stored in long).
    public static Result processFile(String inputPath, String outputPath) throws IOException {
        Path in = Path.of(inputPath);
        Path out = Path.of(outputPath);

        long bytes = 0L;
        long sum = 0L; // unsigned 64-bit sum carried in signed long

        try (InputStream is = new BufferedInputStream(Files.newInputStream(in))) {
            byte[] buf = new byte[8192];
            int n;
            while ((n = is.read(buf)) != -1) {
                bytes += n;
                for (int i = 0; i < n; i++) {
                    sum = (sum + (buf[i] & 0xFFL)) & 0xFFFFFFFFFFFFFFFFL;
                }
            }
        }

        // Write results atomically by writing to a temp file then moving
        Path tmp = out.resolveSibling(out.getFileName().toString() + ".tmp");
        try (BufferedWriter bw = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8,
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
            bw.write("bytes_read=" + Long.toUnsignedString(bytes));
            bw.newLine();
            bw.write("checksum=" + Long.toUnsignedString(sum));
            bw.newLine();
        }
        Files.move(tmp, out, java.nio.file.StandardCopyOption.REPLACE_EXISTING, java.nio.file.StandardCopyOption.ATOMIC_MOVE);

        return new Result(bytes, sum);
    }

    private static void writeBytes(Path p, byte[] data) throws IOException {
        Files.createDirectories(p.getParent() == null ? Path.of(".") : p.getParent());
        Files.write(p, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        Path base = Files.createTempDirectory("lse62_java_tests");
        // Test inputs
        Path in1 = base.resolve("in1_empty.bin");
        Path in2 = base.resolve("in2_hello.txt");
        Path in3 = base.resolve("in3_zeros.bin");
        Path in4 = base.resolve("in4_pattern.bin");
        Path in5 = base.resolve("in5_mixed.bin");

        writeBytes(in1, new byte[0]);
        writeBytes(in2, "hello".getBytes(StandardCharsets.UTF_8));
        writeBytes(in3, new byte[1024]); // 1024 zero bytes
        // pattern spanning multiple buffers
        byte[] pattern = new byte[100000];
        for (int i = 0; i < pattern.length; i++) pattern[i] = (byte) (i & 0xFF);
        writeBytes(in4, pattern);
        // mixed bytes
        byte[] mixed = new byte[]{(byte)0xFF, 0x01, 0x02, 0x7F, (byte)0x80, 0x55, 0x00, (byte)0xAA};
        writeBytes(in5, mixed);

        Path out1 = base.resolve("out1.txt");
        Path out2 = base.resolve("out2.txt");
        Path out3 = base.resolve("out3.txt");
        Path out4 = base.resolve("out4.txt");
        Path out5 = base.resolve("out5.txt");

        Result r1 = processFile(in1.toString(), out1.toString());
        Result r2 = processFile(in2.toString(), out2.toString());
        Result r3 = processFile(in3.toString(), out3.toString());
        Result r4 = processFile(in4.toString(), out4.toString());
        Result r5 = processFile(in5.toString(), out5.toString());

        System.out.println("Test1 bytes=" + Long.toUnsignedString(r1.bytesRead) + " checksum=" + Long.toUnsignedString(r1.checksum) + " -> " + out1);
        System.out.println("Test2 bytes=" + Long.toUnsignedString(r2.bytesRead) + " checksum=" + Long.toUnsignedString(r2.checksum) + " -> " + out2);
        System.out.println("Test3 bytes=" + Long.toUnsignedString(r3.bytesRead) + " checksum=" + Long.toUnsignedString(r3.checksum) + " -> " + out3);
        System.out.println("Test4 bytes=" + Long.toUnsignedString(r4.bytesRead) + " checksum=" + Long.toUnsignedString(r4.checksum) + " -> " + out4);
        System.out.println("Test5 bytes=" + Long.toUnsignedString(r5.bytesRead) + " checksum=" + Long.toUnsignedString(r5.checksum) + " -> " + out5);
    }
}