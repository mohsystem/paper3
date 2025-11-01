
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
        // Test case 1: String with trailing spaces
        System.out.println("Test 1:");
        processAndPrint("Hello World   ");
        
        // Test case 2: String with trailing tabs
        System.out.println("\\nTest 2:");
        processAndPrint("Hello World\\t\\t");
        
        // Test case 3: String with mixed trailing whitespace
        System.out.println("\\nTest 3:");
        processAndPrint("Hello World \\t \\n");
        
        // Test case 4: String with no trailing whitespace
        System.out.println("\\nTest 4:");
        processAndPrint("Hello World");
        
        // Test case 5: Empty string
        System.out.println("\\nTest 5:");
        processAndPrint("");
        
        // If command line arguments are provided
        if (args.length > 0) {
            System.out.println("\\nCommand line argument:");
            processAndPrint(args[0]);
        }
    }
}
