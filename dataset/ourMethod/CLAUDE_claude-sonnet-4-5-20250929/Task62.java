
import java.util.*;
import java.nio.charset.StandardCharsets;

public class Task62 {
    
    public static String parseAndGetRoot(String jsonString) {
        if (jsonString == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        
        if (jsonString.length() > 1000000) {
            throw new IllegalArgumentException("Input too large");
        }
        
        String trimmed = jsonString.trim();
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException("Empty JSON string");
        }
        
        char firstChar = trimmed.charAt(0);
        
        if (firstChar == '{') {
            return "object";
        } else if (firstChar == '[') {
            return "array";
        } else if (trimmed.startsWith("\\"") && trimmed.endsWith("\\"")) {
            return "string";
        } else if (trimmed.equals("true") || trimmed.equals("false")) {
            return "boolean";
        } else if (trimmed.equals("null")) {
            return "null";
        } else if (isValidNumber(trimmed)) {
            return "number";
        } else {
            throw new IllegalArgumentException("Invalid JSON format");
        }
    }
    
    private static boolean isValidNumber(String str) {
        if (str == null || str.isEmpty()) {
            return false;
        }
        
        try {
            Double.parseDouble(str);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        try {
            System.out.println("Test 1: " + parseAndGetRoot("{\\"name\\":\\"John\\",\\"age\\":30}"));
            System.out.println("Test 2: " + parseAndGetRoot("[1,2,3,4,5]"));
            System.out.println("Test 3: " + parseAndGetRoot("\\"Hello World\\""));
            System.out.println("Test 4: " + parseAndGetRoot("42"));
            System.out.println("Test 5: " + parseAndGetRoot("true"));
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
