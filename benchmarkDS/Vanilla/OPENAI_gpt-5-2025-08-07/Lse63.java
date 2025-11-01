import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;

public class Lse63 {
    public static class Result {
        public long bytesRead;
        public long checksum; // 32-bit unsigned stored in long

        public Result(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }

        @Override
        public String toString() {
            return "Result{bytesRead=" + bytesRead + ", checksum=" + checksum + "}";
        }
    }

    public static Result processFile(String inputPath, String outputPath) throws IOException {
        byte[] data = Files.readAllBytes(Paths.get(inputPath));
        long bytesRead = data.length;
        long checksum = 0L;
        for (byte b : data) {
            checksum = (checksum + (b & 0xFF)) & 0xFFFFFFFFL;
        }
        String content = "bytes=" + bytesRead + System.lineSeparator() + "checksum=" + checksum + System.lineSeparator();
        Files.write(Paths.get(outputPath), content.getBytes(StandardCharsets.UTF_8), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        return new Result(bytesRead, checksum);
    }

    private static void writeBytes(String path, byte[] data) throws IOException {
        Files.write(Paths.get(path), data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
    }

    public static void main(String[] args) throws Exception {
        // Test case 1: Empty file
        String in1 = "java_in1.bin";
        String out1 = "java_out1.txt";
        writeBytes(in1, new byte[0]);
        Result r1 = processFile(in1, out1);
        System.out.println("Test1 -> " + r1);

        // Test case 2: "Hello, World!"
        String in2 = "java_in2.bin";
        String out2 = "java_out2.txt";
        writeBytes(in2, "Hello, World!".getBytes(StandardCharsets.UTF_8));
        Result r2 = processFile(in2, out2);
        System.out.println("Test2 -> " + r2);

        // Test case 3: bytes 1..100
        String in3 = "java_in3.bin";
        String out3 = "java_out3.txt";
        byte[] seq = new byte[100];
        for (int i = 0; i < 100; i++) seq[i] = (byte) (i + 1);
        writeBytes(in3, seq);
        Result r3 = processFile(in3, out3);
        System.out.println("Test3 -> " + r3);

        // Test case 4: 10 zero bytes
        String in4 = "java_in4.bin";
        String out4 = "java_out4.txt";
        writeBytes(in4, new byte[10]);
        Result r4 = processFile(in4, out4);
        System.out.println("Test4 -> " + r4);

        // Test case 5: "abc" repeated 1000 times
        String in5 = "java_in5.bin";
        String out5 = "java_out5.txt";
        byte[] rep = "abc".repeat(1000).getBytes(StandardCharsets.UTF_8);
        writeBytes(in5, rep);
        Result r5 = processFile(in5, out5);
        System.out.println("Test5 -> " + r5);
    }
}