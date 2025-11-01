import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;

public class Lse118 {
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    public static String generateAndWrite(String filePath) {
        if (filePath == null) {
            throw new IllegalArgumentException("filePath cannot be null");
        }

        float f1 = SECURE_RANDOM.nextFloat();
        float f2 = SECURE_RANDOM.nextFloat();
        float f3 = SECURE_RANDOM.nextFloat();

        String concatenated = Float.toString(f1) + "," + Float.toString(f2) + "," + Float.toString(f3);

        Path path = Paths.get(filePath).toAbsolutePath().normalize();
        try {
            Files.write(path, concatenated.getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        } catch (IOException e) {
            throw new RuntimeException("Failed to write to file: " + path, e);
        }

        return concatenated;
    }

    public static void main(String[] args) {
        String[] files = {
            "java_out_1.txt",
            "java_out_2.txt",
            "java_out_3.txt",
            "java_out_4.txt",
            "java_out_5.txt"
        };

        for (String f : files) {
            String result = generateAndWrite(f);
            System.out.println("Wrote to " + f + ": " + result);
        }
    }
}