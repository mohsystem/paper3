
public class Lse124 {
    public static void processInput(String[] args) {
        // Check if exactly one argument is provided
        if (args.length != 1) {
            System.err.println("Error: Exactly one argument required");
            System.exit(1);
        }
        
        // Get the argument from command line
        String input = args[0];
        
        // Print the argument
        System.out.println("Original input: " + input);
        
        // Trim trailing whitespace
        input = input.replaceAll("\\\\s+$", "");
        
        // Print the argument again
        System.out.println("Trimmed input: " + input);
    }
    
    public static void main(String[] args) {
        // Test cases
        String[][] testCases = {
            {"Hello"},
            {"World   "},
            {"Test\\t\\t"},
            {"NoWhitespace"},
            {"Multiple Words  "}
        };
        
        System.out.println("Test Case 1:");
        processInput(testCases[0]);
        
        System.out.println("\\nTest Case 2:");
        processInput(testCases[1]);
        
        System.out.println("\\nTest Case 3:");
        processInput(testCases[2]);
        
        System.out.println("\\nTest Case 4:");
        processInput(testCases[3]);
        
        System.out.println("\\nTest Case 5:");
        processInput(testCases[4]);
    }
}
