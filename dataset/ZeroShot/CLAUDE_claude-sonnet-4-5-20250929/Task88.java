
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.SecureRandom;

public class Task88 {
    private static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final SecureRandom RANDOM = new SecureRandom();
    
    public static String createTempFileWithUnicode(int stringLength) throws IOException {
        // Generate random string
        String randomString = generateRandomString(stringLength);
        
        // Convert string to Unicode (Java strings are already Unicode)
        String unicodeString = convertToUnicode(randomString);
        
        // Create temporary file with UTF-8 encoding
        Path tempFile = Files.createTempFile("temp_", ".txt");
        
        // Write Unicode string to file using UTF-8 encoding
        try (BufferedWriter writer = new BufferedWriter(
                new OutputStreamWriter(new FileOutputStream(tempFile.toFile()), StandardCharsets.UTF_8))) {
            writer.write(unicodeString);
        }
        
        return tempFile.toAbsolutePath().toString();
    }
    
    private static String generateRandomString(int length) {
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append(CHARACTERS.charAt(RANDOM.nextInt(CHARACTERS.length())));
        }
        return sb.toString();
    }
    
    private static String convertToUnicode(String input) {
        StringBuilder unicode = new StringBuilder();
        for (char c : input.toCharArray()) {
            unicode.append(String.format("\\\\u%04x", (int) c));
        }
        return unicode.toString();
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1
            String path1 = createTempFileWithUnicode(10);
            System.out.println("Test 1 - Temp file created: " + path1);
            
            // Test case 2
            String path2 = createTempFileWithUnicode(20);
            System.out.println("Test 2 - Temp file created: " + path2);
            
            // Test case 3
            String path3 = createTempFileWithUnicode(5);
            System.out.println("Test 3 - Temp file created: " + path3);
            
            // Test case 4
            String path4 = createTempFileWithUnicode(15);
            System.out.println("Test 4 - Temp file created: " + path4);
            
            // Test case 5
            String path5 = createTempFileWithUnicode(25);
            System.out.println("Test 5 - Temp file created: " + path5);
            
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
