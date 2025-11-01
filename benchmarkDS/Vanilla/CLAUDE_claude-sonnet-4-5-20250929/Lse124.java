
public class Lse124 {
    public static void processInput(String[] args) {
        // Check if exactly one argument is provided
        if (args.length != 1) {
            System.out.println("Error: Please provide exactly one argument");
            System.exit(1);
        }
        
        // Get the argument from command line
        String input = args[0];
        
        // Print the argument
        System.out.println("Original input: " + input);
        
        // Trim the trailing whitespace
        input = input.replaceAll("\\\\s+$", "");
        
        // Print the argument again
        System.out.println("Trimmed input: " + input);
        
        // Exit the program
        System.exit(0);
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string with trailing spaces
        System.out.println("Test Case 1:");
        processInput(new String[]{"Hello World   "});
        
        System.out.println("\\nTest Case 2:");
        processInput(new String[]{"Test123\\t\\t"});
        
        System.out.println("\\nTest Case 3:");
        processInput(new String[]{"NoTrailingSpace"});
        
        System.out.println("\\nTest Case 4:");
        processInput(new String[]{"Multiple   Spaces   "});
        
        System.out.println("\\nTest Case 5:");
        processInput(new String[]{"   Leading and trailing   "});
    }
}
