
import java.util.*;
import com.google.gson.*;

public class Task62 {
    public static JsonElement parseAndGetRoot(String jsonString) {
        if (jsonString == null || jsonString.trim().isEmpty()) {
            return null;
        }
        
        try {
            JsonParser parser = new JsonParser();
            JsonElement rootElement = parser.parse(jsonString);
            return rootElement;
        } catch (JsonSyntaxException e) {
            System.err.println("Invalid JSON syntax: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple JSON object
        String test1 = "{\\"name\\": \\"John\\", \\"age\\": 30}";
        System.out.println("Test 1 Root: " + parseAndGetRoot(test1));
        
        // Test case 2: JSON array
        String test2 = "[1, 2, 3, 4, 5]";
        System.out.println("Test 2 Root: " + parseAndGetRoot(test2));
        
        // Test case 3: Nested JSON object
        String test3 = "{\\"user\\": {\\"name\\": \\"Alice\\", \\"address\\": {\\"city\\": \\"NYC\\"}}}";
        System.out.println("Test 3 Root: " + parseAndGetRoot(test3));
        
        // Test case 4: Empty JSON object
        String test4 = "{}";
        System.out.println("Test 4 Root: " + parseAndGetRoot(test4));
        
        // Test case 5: Invalid JSON (should handle gracefully)
        String test5 = "{invalid json}";
        System.out.println("Test 5 Root: " + parseAndGetRoot(test5));
    }
}
