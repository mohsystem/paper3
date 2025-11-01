
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

public class Lse48 {
    public static byte[] img(Map<String, String> requestArgs) {
        try {
            String file = requestArgs.get("file");
            if (file == null || file.isEmpty()) {
                return null;
            }
            
            Path filePath = Paths.get(file);
            return Files.readAllBytes(filePath);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid image file
        Map<String, String> test1 = new HashMap<>();
        test1.put("file", "test1.jpg");
        byte[] result1 = img(test1);
        System.out.println("Test 1 - File: test1.jpg, Result: " + (result1 != null ? "Success (" + result1.length + " bytes)" : "Failed"));
        
        // Test case 2: Valid text file
        Map<String, String> test2 = new HashMap<>();
        test2.put("file", "test2.txt");
        byte[] result2 = img(test2);
        System.out.println("Test 2 - File: test2.txt, Result: " + (result2 != null ? "Success (" + result2.length + " bytes)" : "Failed"));
        
        // Test case 3: Empty file parameter
        Map<String, String> test3 = new HashMap<>();
        test3.put("file", "");
        byte[] result3 = img(test3);
        System.out.println("Test 3 - Empty file, Result: " + (result3 != null ? "Success" : "Failed (Expected)"));
        
        // Test case 4: Null file parameter
        Map<String, String> test4 = new HashMap<>();
        byte[] result4 = img(test4);
        System.out.println("Test 4 - Null file, Result: " + (result4 != null ? "Success" : "Failed (Expected)"));
        
        // Test case 5: Non-existent file
        Map<String, String> test5 = new HashMap<>();
        test5.put("file", "nonexistent.jpg");
        byte[] result5 = img(test5);
        System.out.println("Test 5 - Non-existent file, Result: " + (result5 != null ? "Success" : "Failed (Expected)"));
    }
}
