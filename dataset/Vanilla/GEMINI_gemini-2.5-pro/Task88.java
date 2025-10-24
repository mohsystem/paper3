import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Random;

public class Task88 {

    private static final String UNICODE_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" +
                                                "àáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ" +
                                                "ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ" +
                                                "αβγδεζηθικλμνξοπρστυφχψω" +
                                                "€£¥©®™";

    private static String generateRandomUnicodeString(int length) {
        StringBuilder sb = new StringBuilder(length);
        Random random = new Random();
        for (int i = 0; i < length; i++) {
            int index = random.nextInt(UNICODE_CHARS.length());
            sb.append(UNICODE_CHARS.charAt(index));
        }
        return sb.toString();
    }

    /**
     * Creates a temporary file using UTF-8 encoding, writes a random Unicode string to it,
     * and returns the path of the temporary file.
     *
     * @return The absolute path to the created temporary file, or null on failure.
     */
    public static String createTempFileWithRandomUnicodeString() {
        try {
            // 1. Create a temporary file.
            Path tempFile = Files.createTempFile("temp_unicode_", ".txt");

            // 2. Create a random string.
            String randomString = generateRandomUnicodeString(100);
            
            // 3. In Java, strings are internally Unicode (UTF-16). We write them out using UTF-8.
            // 4. Write the string to the file using UTF-8 encoding.
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
                writer.write(randomString);
            }
            
            // 5. Return the path of the temporary file.
            return tempFile.toAbsolutePath().toString();

        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            System.out.println("Test Case " + i + ":");
            String filePath = createTempFileWithRandomUnicodeString();
            if (filePath != null) {
                System.out.println("  Temporary file created at: " + filePath);
                try {
                    Path path = Path.of(filePath);
                    // Optional: Read back to verify content was written correctly.
                    String content = Files.readString(path, StandardCharsets.UTF_8);
                    System.out.println("  File content length: " + content.length());
                    System.out.println("  File content starts with: " + (content.length() > 20 ? content.substring(0, 20) + "..." : content));
                    
                    // Cleanup the temporary file.
                    Files.delete(path);
                    System.out.println("  Temporary file deleted.");
                } catch (IOException e) {
                    System.err.println("  Error during file verification or cleanup: " + e.getMessage());
                }
            } else {
                System.out.println("  Failed to create temporary file.");
            }
            System.out.println();
        }
    }
}