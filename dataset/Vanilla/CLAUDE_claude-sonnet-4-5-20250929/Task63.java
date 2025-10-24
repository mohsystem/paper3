
import java.util.Base64;
import java.nio.charset.StandardCharsets;
import org.json.JSONObject;
import org.json.JSONException;

public class Task63 {
    public static String processRequest(String rawData) {
        try {
            // Step 1: Ensure raw_data is encoded in UTF-8 (already String in Java)
            byte[] rawDataBytes = rawData.getBytes(StandardCharsets.UTF_8);
            
            // Step 2: Decode the raw_data using Base64
            byte[] decodedBytes = Base64.getDecoder().decode(rawDataBytes);
            
            // Step 3: Convert decoded bytes to String
            String decodedData = new String(decodedBytes, StandardCharsets.UTF_8);
            
            // Step 4: Deserialize the decoded data (assuming JSON format)
            // Store the deserialized data
            JSONObject jsonObject = new JSONObject(decodedData);
            String storedData = jsonObject.toString();
            
            // Step 5: Return the stored data
            return storedData;
        } catch (Exception e) {
            return "Error processing request: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple JSON object
        String test1 = Base64.getEncoder().encodeToString("{\\"name\\":\\"John\\",\\"age\\":30}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 1: " + processRequest(test1));
        
        // Test case 2: JSON with nested object
        String test2 = Base64.getEncoder().encodeToString("{\\"user\\":{\\"id\\":1,\\"name\\":\\"Alice\\"}}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 2: " + processRequest(test2));
        
        // Test case 3: JSON with array
        String test3 = Base64.getEncoder().encodeToString("{\\"items\\":[1,2,3,4,5]}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 3: " + processRequest(test3));
        
        // Test case 4: JSON with boolean and null
        String test4 = Base64.getEncoder().encodeToString("{\\"active\\":true,\\"data\\":null}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 4: " + processRequest(test4));
        
        // Test case 5: Complex JSON
        String test5 = Base64.getEncoder().encodeToString("{\\"company\\":\\"TechCorp\\",\\"employees\\":[{\\"name\\":\\"Bob\\",\\"role\\":\\"Developer\\"}]}".getBytes(StandardCharsets.UTF_8));
        System.out.println("Test 5: " + processRequest(test5));
    }
}
