import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.charset.StandardCharsets;

public class Lse62 {

    public static long[] processFile(String inputPath, String outputPath) throws IOException {
        byte[] data = Files.readAllBytes(Path.of(inputPath));
        long bytesRead = data.length;
        long checksum = 0L;
        for (byte b : data) {
            checksum = (checksum + (b & 0xFF)) & 0xFFFFFFFFL;
        }
        String content = "bytes=" + bytesRead + System.lineSeparator() + "checksum=" + checksum + System.lineSeparator();
        Files.write(Path.of(outputPath), content.getBytes(StandardCharsets.UTF_8));
        return new long[]{bytesRead, checksum};
    }

    private static void writeBytes(String path, byte[] data) throws IOException {
        Files.write(Path.of(path), data);
    }

    public static void main(String[] args) throws Exception {
        // Test case 1: Empty file
        String in1 = "java_in1.bin";
        String out1 = "java_out1.txt";
        writeBytes(in1, new byte[0]);
        long[] r1 = processFile(in1, out1);
        System.out.println("Test 1 -> bytes=" + r1[0] + ", checksum=" + r1[1] + ", saved: " + out1);

        // Test case 2: "Hello, world!"
        String in2 = "java_in2.bin";
        String out2 = "java_out2.txt";
        writeBytes(in2, "Hello, world!".getBytes(StandardCharsets.UTF_8));
        long[] r2 = processFile(in2, out2);
        System.out.println("Test 2 -> bytes=" + r2[0] + ", checksum=" + r2[1] + ", saved: " + out2);

        // Test case 3: 1024 bytes 0..255 repeating
        String in3 = "java_in3.bin";
        String out3 = "java_out3.txt";
        byte[] data3 = new byte[1024];
        for (int i = 0; i < data3.length; i++) data3[i] = (byte) (i & 0xFF);
        writeBytes(in3, data3);
        long[] r3 = processFile(in3, out3);
        System.out.println("Test 3 -> bytes=" + r3[0] + ", checksum=" + r3[1] + ", saved: " + out3);

        // Test case 4: Non-ASCII pattern repeated
        String in4 = "java_in4.bin";
        String out4 = "java_out4.txt";
        byte[] pattern4 = new byte[]{0x00, (byte) 0xFF, 0x10, 0x7F, (byte) 0x80};
        byte[] data4 = new byte[100];
        for (int i = 0; i < data4.length; i++) data4[i] = pattern4[i % pattern4.length];
        writeBytes(in4, data4);
        long[] r4 = processFile(in4, out4);
        System.out.println("Test 4 -> bytes=" + r4[0] + ", checksum=" + r4[1] + ", saved: " + out4);

        // Test case 5: 4096 bytes deterministic generator
        String in5 = "java_in5.bin";
        String out5 = "java_out5.txt";
        byte[] data5 = new byte[4096];
        for (int i = 0; i < data5.length; i++) data5[i] = (byte) ((i * 31 + 7) & 0xFF);
        writeBytes(in5, data5);
        long[] r5 = processFile(in5, out5);
        System.out.println("Test 5 -> bytes=" + r5[0] + ", checksum=" + r5[1] + ", saved: " + out5);
    }
}