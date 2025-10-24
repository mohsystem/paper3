
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.Map;
import java.util.HashMap;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.JsonNode;

public class Task63 {
    private static final int MAX_INPUT_SIZE = 1024 * 1024; // 1MB limit
    
    public static JsonNode processRequest(String rawData) throws Exception {
        if (rawData == null || rawData.isEmpty()) {
            throw new IllegalArgumentException("Raw data cannot be null or empty");
        }
        
        if (rawData.length() > MAX_INPUT_SIZE) {
            throw new IllegalArgumentException("Input exceeds maximum size limit");
        }
        
        // Ensure UTF-8 encoding
        byte[] utf8Bytes = rawData.getBytes(StandardCharsets.UTF_8);
        String utf8String = new String(utf8Bytes, StandardCharsets.UTF_8);
        
        // Decode Base64
        byte[] decodedBytes;
        try {
            decodedBytes = Base64.getDecoder().decode(utf8String);
        } catch (IllegalArgumentException e) {
            throw new IllegalArgumentException("Invalid Base64 input", e);
        }
        
        if (decodedBytes.length > MAX_INPUT_SIZE) {
            throw new IllegalArgumentException("Decoded data exceeds maximum size limit");
        }
        
        // Convert decoded bytes to string
        String decodedString = new String(decodedBytes, StandardCharsets.UTF_8);
        
        // Deserialize JSON safely
        ObjectMapper mapper = new ObjectMapper();
        mapper.enableDefaultTyping(ObjectMapper.DefaultTyping.NON_FINAL);
        mapper.activateDefaultTyping(mapper.getPolymorphicTypeValidator(), ObjectMapper.DefaultTyping.NON_FINAL);
        
        // Disable features that could lead to vulnerabilities
        mapper.configure(com.fasterxml.jackson.databind.DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
        
        JsonNode deserializedData;
        try {
            deserializedData = mapper.readTree(decodedString);
        } catch (Exception e) {
            throw new IllegalArgumentException("Invalid JSON format", e);
        }
        
        return deserializedData;
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Valid JSON object
            String test1 = Base64.getEncoder().encodeToString("{\\"name\\":\\"John\\",\\"age\\":30}".getBytes(StandardCharsets.UTF_8));
            JsonNode result1 = processRequest(test1);
            System.out.println("Test 1: " + result1);
            
            // Test case 2: Valid JSON array
            String test2 = Base64.getEncoder().encodeToString("[1,2,3,4,5]".getBytes(StandardCharsets.UTF_8));
            JsonNode result2 = processRequest(test2);
            System.out.println("Test 2: " + result2);
            
            // Test case 3: Nested JSON
            String test3 = Base64.getEncoder().encodeToString("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}".getBytes(StandardCharsets.UTF_8));
            JsonNode result3 = processRequest(test3);
            System.out.println("Test 3: " + result3);
            
            // Test case 4: Simple string value
            String test4 = Base64.getEncoder().encodeToString("\\"Hello World\\"".getBytes(StandardCharsets.UTF_8));
            JsonNode result4 = processRequest(test4);
            System.out.println("Test 4: " + result4);
            
            // Test case 5: Number value
            String test5 = Base64.getEncoder().encodeToString("42".getBytes(StandardCharsets.UTF_8));
            JsonNode result5 = processRequest(test5);
            System.out.println("Test 5: " + result5);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
