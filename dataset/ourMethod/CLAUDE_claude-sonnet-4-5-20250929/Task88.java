
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.SecureRandom;

public class Task88 {
    private static final int RANDOM_STRING_LENGTH = 32;
    private static final String CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    public static String createRandomString(int length) {
        if (length <= 0 || length > 10000) {
            throw new IllegalArgumentException("Length must be between 1 and 10000");
        }
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append(CHARS.charAt(SECURE_RANDOM.nextInt(CHARS.length())));
        }
        return sb.toString();
    }

    public static String convertToUnicode(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null");
        }
        StringBuilder unicode = new StringBuilder();
        for (char c : input.toCharArray()) {
            unicode.append(String.format("\\\\u%04x", (int) c));
        }
        return unicode.toString();
    }

    public static Path createTempFileWithUnicode() throws IOException {
        String randomString = createRandomString(RANDOM_STRING_LENGTH);
        String unicodeString = convertToUnicode(randomString);
        
        Path tempFile = Files.createTempFile("temp_", ".txt");
        tempFile.toFile().deleteOnExit();
        
        try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
            writer.write(unicodeString);
            writer.flush();
        }
        
        return tempFile;
    }

    public static void main(String[] args) {
        try {
            // Test case 1
            Path path1 = createTempFileWithUnicode();
            System.out.println("Test 1 - Temp file created: " + path1.toString());
            
            // Test case 2
            Path path2 = createTempFileWithUnicode();
            System.out.println("Test 2 - Temp file created: " + path2.toString());
            
            // Test case 3
            Path path3 = createTempFileWithUnicode();
            System.out.println("Test 3 - Temp file created: " + path3.toString());
            
            // Test case 4
            Path path4 = createTempFileWithUnicode();
            System.out.println("Test 4 - Temp file created: " + path4.toString());
            
            // Test case 5
            Path path5 = createTempFileWithUnicode();
            System.out.println("Test 5 - Temp file created: " + path5.toString());
            
        } catch (IOException e) {
            System.err.println("Error creating temp file: " + e.getMessage());
        }
    }
}
