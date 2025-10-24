
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import java.util.Map;

public class Task63 {
    private static final int MAX_DATA_SIZE = 1024 * 1024; // 1MB limit
    private static final Gson gson = new Gson();
    
    public static Object processRequest(String rawData) {
        if (rawData == null || rawData.isEmpty()) {
            throw new IllegalArgumentException("Raw data cannot be null or empty");
        }
        
        // Validate data size to prevent DoS attacks
        if (rawData.length() > MAX_DATA_SIZE) {
            throw new IllegalArgumentException("Data exceeds maximum allowed size");
        }
        
        try {
            // Ensure UTF-8 encoding
            byte[] utf8Bytes = rawData.getBytes(StandardCharsets.UTF_8);
            
            // Decode Base64 with validation
            byte[] decodedBytes = Base64.getDecoder().decode(utf8Bytes);
            
            // Validate decoded size
            if (decodedBytes.length > MAX_DATA_SIZE) {
                throw new IllegalArgumentException("Decoded data exceeds maximum allowed size");
            }
            
            // Convert to UTF-8 string
            String decodedString = new String(decodedBytes, StandardCharsets.UTF_8);
            
            // Sanitize and validate JSON before deserialization
            if (!isValidJson(decodedString)) {
                throw new IllegalArgumentException("Invalid JSON format");
            }
            
            // Deserialize JSON safely
            Object deserializedData = gson.fromJson(decodedString, Map.class);
            
            return deserializedData;
            
        } catch (IllegalArgumentException e) {
            System.err.println("Base64 decoding error: " + e.getMessage());
            throw new RuntimeException("Invalid Base64 encoded data", e);
        } catch (JsonSyntaxException e) {
            System.err.println("JSON deserialization error: " + e.getMessage());
            throw new RuntimeException("Invalid JSON data", e);
        } catch (Exception e) {
            System.err.println("Processing error: " + e.getMessage());
            throw new RuntimeException("Error processing request", e);
        }
    }
    
    private static boolean isValidJson(String jsonString) {
        if (jsonString == null || jsonString.trim().isEmpty()) {
            return false;
        }
        try {
            gson.fromJson(jsonString, Object.class);
            return true;
        } catch (JsonSyntaxException e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Testing Task63 - Secure Request Processing\\n");
        
        // Test Case 1: Valid JSON object
        try {
            String testData1 = Base64.getEncoder().encodeToString(
                "{\\"name\\":\\"John\\",\\"age\\":30}".getBytes(StandardCharsets.UTF_8));
            Object result1 = processRequest(testData1);
            System.out.println("Test 1 - Valid JSON: " + result1);
        } catch (Exception e) {
            System.out.println("Test 1 Failed: " + e.getMessage());
        }
        
        // Test Case 2: Valid JSON array
        try {
            String testData2 = Base64.getEncoder().encodeToString(
                "[\\"item1\\",\\"item2\\",\\"item3\\"]".getBytes(StandardCharsets.UTF_8));
            Object result2 = processRequest(testData2);
            System.out.println("Test 2 - Valid JSON Array: " + result2);
        } catch (Exception e) {
            System.out.println("Test 2 Failed: " + e.getMessage());
        }
        
        // Test Case 3: Nested JSON
        try {
            String testData3 = Base64.getEncoder().encodeToString(
                "{\\"user\\":{\\"id\\":123,\\"status\\":\\"active\\"}}".getBytes(StandardCharsets.UTF_8));
            Object result3 = processRequest(testData3);
            System.out.println("Test 3 - Nested JSON: " + result3);
        } catch (Exception e) {
            System.out.println("Test 3 Failed: " + e.getMessage());
        }
        
        // Test Case 4: Invalid Base64
        try {
            String testData4 = "Invalid@Base64!Data";
            Object result4 = processRequest(testData4);
            System.out.println("Test 4 - Invalid Base64: " + result4);
        } catch (Exception e) {
            System.out.println("Test 4 Failed (Expected): " + e.getMessage());
        }
        
        // Test Case 5: Invalid JSON
        try {
            String testData5 = Base64.getEncoder().encodeToString(
                "{invalid json}".getBytes(StandardCharsets.UTF_8));
            Object result5 = processRequest(testData5);
            System.out.println("Test 5 - Invalid JSON: " + result5);
        } catch (Exception e) {
            System.out.println("Test 5 Failed (Expected): " + e.getMessage());
        }
    }
}
