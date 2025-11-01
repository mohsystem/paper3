import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.security.SecureRandom;

public class Lse118 {
    public static String generateAndWrite(String filePath) throws IOException {
        SecureRandom sr = new SecureRandom();
        float a = sr.nextFloat();
        float b = sr.nextFloat();
        float c = sr.nextFloat();

        String result = Float.toString(a) + Float.toString(b) + Float.toString(c);

        Files.write(
                Path.of(filePath),
                result.getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.CREATE,
                StandardOpenOption.TRUNCATE_EXISTING,
                StandardOpenOption.WRITE
        );

        return result;
    }

    public static void main(String[] args) {
        String[] files = {"java_out1.txt", "java_out2.txt", "java_out3.txt", "java_out4.txt", "java_out5.txt"};
        for (String f : files) {
            try {
                String s = generateAndWrite(f);
                System.out.println("Wrote to " + f + ": " + s);
            } catch (IOException e) {
                System.err.println("Error writing to " + f + ": " + e.getMessage());
            }
        }
    }
}