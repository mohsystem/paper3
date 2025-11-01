
public class Lse123 {
    public static String processString(String[] args) {
        // Check if exactly one argument is provided
        if (args.length != 1) {
            return "Error: Exactly one argument required";
        }
        
        // Check if the argument is a string (already guaranteed in Java)
        String input = args[0];
        
        if (input == null || input.isEmpty()) {
            return "";
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
        
        // Return the trimmed string (null termination is automatic in Java)
        return end >= 0 ? input.substring(0, end + 1) : "";
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string with trailing spaces
        String[] test1 = {"Hello World   "};
        System.out.println("Test 1: '" + processString(test1) + "'");
        
        // Test case 2: String with trailing newlines
        String[] test2 = {"Hello\\n\\n"};
        System.out.println("Test 2: '" + processString(test2) + "'");
        
        // Test case 3: String with both spaces and newlines
        String[] test3 = {"Test String  \\n  \\n"};
        System.out.println("Test 3: '" + processString(test3) + "'");
        
        // Test case 4: Empty string
        String[] test4 = {""};
        System.out.println("Test 4: '" + processString(test4) + "'");
        
        // Test case 5: No trailing whitespace
        String[] test5 = {"Clean"};
        System.out.println("Test 5: '" + processString(test5) + "'");
    }
}
