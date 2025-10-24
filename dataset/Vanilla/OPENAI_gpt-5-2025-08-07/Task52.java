import java.io.*;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task52 {

    public static long encryptFile(String inputPath, String outputPath, String key) throws IOException {
        return xorTransformFile(inputPath, outputPath, key.getBytes(StandardCharsets.UTF_8));
    }

    public static long decryptFile(String inputPath, String outputPath, String key) throws IOException {
        return xorTransformFile(inputPath, outputPath, key.getBytes(StandardCharsets.UTF_8));
    }

    private static long xorTransformFile(String inputPath, String outputPath, byte[] key) throws IOException {
        if (key == null || key.length == 0) {
            throw new IllegalArgumentException("Key must not be empty");
        }
        Path in = Paths.get(inputPath);
        Path out = Paths.get(outputPath);
        Files.createDirectories(out.toAbsolutePath().getParent() != null ? out.toAbsolutePath().getParent() : Paths.get("."));
        long total = 0;
        try (InputStream is = Files.newInputStream(in);
             OutputStream os = Files.newOutputStream(out)) {
            byte[] buf = new byte[65536];
            int read;
            int ki = 0;
            while ((read = is.read(buf)) != -1) {
                for (int i = 0; i < read; i++) {
                    buf[i] = (byte) (buf[i] ^ key[ki]);
                    ki++;
                    if (ki == key.length) ki = 0;
                }
                os.write(buf, 0, read);
                total += read;
            }
        }
        return total;
    }

    private static void writeBytes(String path, byte[] data) throws IOException {
        Files.write(Paths.get(path), data);
    }

    private static byte[] readBytes(String path) throws IOException {
        return Files.readAllBytes(Paths.get(path));
    }

    private static boolean filesEqual(String p1, String p2) throws IOException {
        return Arrays.equals(readBytes(p1), readBytes(p2));
    }

    public static void main(String[] args) {
        try {
            // Test case 1: simple text
            String in1 = "java_in1.txt", enc1 = "java_in1.enc", dec1 = "java_in1.dec";
            writeBytes(in1, "Hello, World!".getBytes(StandardCharsets.UTF_8));
            encryptFile(in1, enc1, "key1");
            decryptFile(enc1, dec1, "key1");
            System.out.println("Test1 OK: " + filesEqual(in1, dec1));

            // Test case 2: unicode text
            String in2 = "java_in2.txt", enc2 = "java_in2.enc", dec2 = "java_in2.dec";
            writeBytes(in2, "こんにちは世界 🌍🚀".getBytes(StandardCharsets.UTF_8));
            encryptFile(in2, enc2, "日本語の鍵");
            decryptFile(enc2, dec2, "日本語の鍵");
            System.out.println("Test2 OK: " + filesEqual(in2, dec2));

            // Test case 3: larger content
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 20000; i++) sb.append("Line ").append(i).append(" - The quick brown fox jumps over the lazy dog.\n");
            String in3 = "java_in3.txt", enc3 = "java_in3.enc", dec3 = "java_in3.dec";
            writeBytes(in3, sb.toString().getBytes(StandardCharsets.UTF_8));
            encryptFile(in3, enc3, "longersecretkey123");
            decryptFile(enc3, dec3, "longersecretkey123");
            System.out.println("Test3 OK: " + filesEqual(in3, dec3));

            // Test case 4: binary data
            byte[] bin = new byte[1024];
            for (int i = 0; i < bin.length; i++) bin[i] = (byte) (i & 0xFF);
            String in4 = "java_in4.bin", enc4 = "java_in4.enc", dec4 = "java_in4.dec";
            writeBytes(in4, bin);
            encryptFile(in4, enc4, "binKey\x00\x01\x02");
            decryptFile(enc4, dec4, "binKey\x00\x01\x02");
            System.out.println("Test4 OK: " + filesEqual(in4, dec4));

            // Test case 5: empty file
            String in5 = "java_in5.empty", enc5 = "java_in5.enc", dec5 = "java_in5.dec";
            writeBytes(in5, new byte[0]);
            encryptFile(in5, enc5, "emptyKey");
            decryptFile(enc5, dec5, "emptyKey");
            System.out.println("Test5 OK: " + filesEqual(in5, dec5));

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}