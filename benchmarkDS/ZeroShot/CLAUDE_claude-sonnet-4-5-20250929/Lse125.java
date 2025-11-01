
public class Lse125 {
    public static String processAndPrint(String input) {
        if (input == null) {
            return "";
        }
        String trimmed = input.stripTrailing();
        System.out.println(trimmed);
        return trimmed;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string with trailing spaces
        System.out.println("Test 1:");
        processAndPrint("Hello World   ");
        
        // Test case 2: String with trailing tabs and spaces
        System.out.println("\\nTest 2:");
        processAndPrint("Java Programming\\t\\t  ");
        
        // Test case 3: String with no trailing whitespace
        System.out.println("\\nTest 3:");
        processAndPrint("No trailing space");
        
        // Test case 4: String with only whitespace
        System.out.println("\\nTest 4:");
        processAndPrint("    ");
        
        // Test case 5: Empty string
        System.out.println("\\nTest 5:");
        processAndPrint("");
        
        // Process command line arguments if provided
        if (args.length > 0) {
            System.out.println("\\nCommand line argument:");
            processAndPrint(args[0]);
        }
    }
}
