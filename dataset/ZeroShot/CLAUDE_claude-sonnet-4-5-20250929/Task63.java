
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.util.Map;

public class Task63 {
    
    public static Object processRequest(String rawData) {
        try {
            // Step 1 & 2: Ensure UTF-8 encoding and decode Base64
            byte[] decodedBytes = Base64.getDecoder().decode(rawData.getBytes(StandardCharsets.UTF_8));
            
            // Step 3: Deserialize the decoded data (assuming JSON format)
            String decodedString = new String(decodedBytes, StandardCharsets.UTF_8);
            ObjectMapper objectMapper = new ObjectMapper();
            Object deserializedData = objectMapper.readValue(decodedString, Object.class);
            
            // Step 4 & 5: Store and return the deserialized data
            return deserializedData;
            
        } catch (Exception e) {
            System.err.println("Error processing request: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple JSON object
        String test1 = Base64.getEncoder().encodeToString("{\\"name\\":\\"John\\",\\"age\\":30}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 1: " + processRequest(test1));
        
        // Test Case 2: JSON array
        String test2 = Base64.getEncoder().encodeToString("[1,2,3,4,5]".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 2: " + processRequest(test2));
        
        // Test Case 3: Nested JSON
        String test3 = Base64.getEncoder().encodeToString("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 3: " + processRequest(test3));
        
        // Test Case 4: JSON with special characters
        String test4 = Base64.getEncoder().encodeToString("{\\"message\\":\\"Hello, World!\\"}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 4: " + processRequest(test4));
        
        // Test Case 5: Empty JSON object
        String test5 = Base64.getEncoder().encodeToString("{}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 5: " + processRequest(test5));
    }
}
