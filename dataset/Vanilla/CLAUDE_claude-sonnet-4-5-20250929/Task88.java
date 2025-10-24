
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Random;

public class Task88 {
    public static String createTempFileWithUnicode(int stringLength) throws IOException {
        // Create a random string
        String randomString = generateRandomString(stringLength);
        
        // Convert string to Unicode (Java strings are already Unicode)
        String unicodeString = convertToUnicode(randomString);
        
        // Create temporary file with UTF-8 encoding
        Path tempFile = Files.createTempFile("temp_", ".txt");
        
        // Write Unicode string to file using UTF-8 encoding
        Files.write(tempFile, unicodeString.getBytes(StandardCharsets.UTF_8));
        
        return tempFile.toAbsolutePath().toString();
    }
    
    private static String generateRandomString(int length) {
        String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        Random random = new Random();
        StringBuilder sb = new StringBuilder(length);
        
        for (int i = 0; i < length; i++) {
            sb.append(chars.charAt(random.nextInt(chars.length())));
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
            // Test case 1: Create temp file with 10 character string
            System.out.println("Test Case 1:");
            String path1 = createTempFileWithUnicode(10);
            System.out.println("Temp file created at: " + path1);
            System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(path1)), StandardCharsets.UTF_8));
            System.out.println();
            
            // Test case 2: Create temp file with 20 character string
            System.out.println("Test Case 2:");
            String path2 = createTempFileWithUnicode(20);
            System.out.println("Temp file created at: " + path2);
            System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(path2)), StandardCharsets.UTF_8));
            System.out.println();
            
            // Test case 3: Create temp file with 5 character string
            System.out.println("Test Case 3:");
            String path3 = createTempFileWithUnicode(5);
            System.out.println("Temp file created at: " + path3);
            System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(path3)), StandardCharsets.UTF_8));
            System.out.println();
            
            // Test case 4: Create temp file with 15 character string
            System.out.println("Test Case 4:");
            String path4 = createTempFileWithUnicode(15);
            System.out.println("Temp file created at: " + path4);
            System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(path4)), StandardCharsets.UTF_8));
            System.out.println();
            
            // Test case 5: Create temp file with 25 character string
            System.out.println("Test Case 5:");
            String path5 = createTempFileWithUnicode(25);
            System.out.println("Temp file created at: " + path5);
            System.out.println("Content: " + new String(Files.readAllBytes(Paths.get(path5)), StandardCharsets.UTF_8));
            System.out.println();
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
