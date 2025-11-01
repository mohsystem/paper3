import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;

public class Lse63 {
    public static class Result {
        public final long bytesRead;
        public final long checksum; // unsigned 32-bit stored in long
        public final boolean isEmpty;

        public Result(long bytesRead, long checksum, boolean isEmpty) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
            this.isEmpty = isEmpty;
        }

        @Override
        public String toString() {
            return "Result{bytesRead=" + bytesRead + ", checksum=" + Long.toUnsignedString(checksum) + ", isEmpty=" + isEmpty + "}";
        }
    }

    public static Result processFile(String inputPath, String outputPath) throws IOException {
        Path in = Paths.get(inputPath);
        byte[] data = Files.readAllBytes(in);

        long checksum = 0L;
        for (byte b : data) {
            checksum = (checksum + (b & 0xFF)) & 0xFFFFFFFFL;
        }
        boolean empty = (checksum == 0L);

        Path out = Paths.get(outputPath);
        if (out.getParent() != null) {
            Files.createDirectories(out.getParent());
        }
        try (BufferedWriter w = Files.newBufferedWriter(out, StandardCharsets.UTF_8,
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
            w.write("bytes_read=" + data.length);
            w.newLine();
            w.write("checksum=" + Long.toUnsignedString(checksum));
            w.newLine();
        }

        return new Result(data.length, checksum, empty);
    }

    public static void main(String[] args) {
        try {
            Path tmp = Files.createTempDirectory("lse63_java_demo_");

            String[] names = {"empty.bin", "hello.txt", "repeatA.bin", "binary.bin", "unicode.txt"};
            byte[][] contents = new byte[5][];

            contents[0] = new byte[0];
            contents[1] = "hello".getBytes(StandardCharsets.UTF_8);

            byte[] rep = new byte[1024];
            Arrays.fill(rep, (byte) 'a');
            contents[2] = rep;

            contents[3] = new byte[]{0x01, 0x02, (byte) 0xFF, 0x10, 0x20, 0x7F};
            contents[4] = "こんにちは".getBytes(StandardCharsets.UTF_8);

            for (int i = 0; i < 5; i++) {
                Path in = tmp.resolve(names[i]);
                Files.write(in, contents[i]);
                Path out = tmp.resolve(names[i] + ".out.txt");
                Result r = processFile(in.toString(), out.toString());
                System.out.println("Test " + (i + 1) + ": " + in + " -> " + r);
                System.out.println("Saved: " + out);
            }
            System.out.println("Output dir: " + tmp.toAbsolutePath());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}