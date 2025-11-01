
public class Lse123 {
    public static String processString(String input) {
        if (input == null) {
            return null;
        }
        
        // Trim trailing whitespace
        int end = input.length() - 1;
        while (end >= 0 && Character.isWhitespace(input.charAt(end))) {
            end--;
        }
        
        // Trim newlines
        while (end >= 0 && (input.charAt(end) == '\\n' || input.charAt(end) == '\\r')) {
            end--;
        }
        
        // Return the trimmed string (Java strings are already null-terminated internally)
        return input.substring(0, end + 1);
    }
    
    public static void main(String[] args) {
        // Test case 1: String with trailing spaces
        System.out.println("Test 1: '" + processString("hello world   ") + "'");
        
        // Test case 2: String with trailing newlines
        System.out.println("Test 2: '" + processString("hello world\\n\\n") + "'");
        
        // Test case 3: String with trailing spaces and newlines
        System.out.println("Test 3: '" + processString("hello world  \\n\\r ") + "'");
        
        // Test case 4: String with no trailing whitespace
        System.out.println("Test 4: '" + processString("hello world") + "'");
        
        // Test case 5: Empty string
        System.out.println("Test 5: '" + processString("") + "'");
    }
}
