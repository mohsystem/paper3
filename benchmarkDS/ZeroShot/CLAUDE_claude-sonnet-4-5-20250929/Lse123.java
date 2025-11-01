
public class Lse123 {
    public static String trimString(String input) {
        if (input == null) {
            return null;
        }
        
        // Check if input is a string (already guaranteed in Java)
        if (!(input instanceof String)) {
            return null;
        }
        
        // Trim trailing whitespace
        int endIndex = input.length() - 1;
        while (endIndex >= 0 && Character.isWhitespace(input.charAt(endIndex))) {
            endIndex--;
        }
        
        // Trim newlines from the end
        while (endIndex >= 0 && (input.charAt(endIndex) == '\\n' || input.charAt(endIndex) == '\\r')) {
            endIndex--;
        }
        
        // Return the trimmed string (null termination is automatic in Java)
        return input.substring(0, endIndex + 1);
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: '" + trimString("hello world   ") + "'");
        System.out.println("Test 2: '" + trimString("test\\n\\n") + "'");
        System.out.println("Test 3: '" + trimString("spaces and newlines  \\n\\n  ") + "'");
        System.out.println("Test 4: '" + trimString("no trailing") + "'");
        System.out.println("Test 5: '" + trimString("   \\n\\n") + "'");
    }
}
