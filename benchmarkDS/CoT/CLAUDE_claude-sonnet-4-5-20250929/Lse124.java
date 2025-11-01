
public class Lse124 {
    public static void main(String[] args) {
        // Check if exactly one argument is provided
        if (args.length != 1) {
            System.err.println("Error: Exactly one argument is required.");
            System.exit(1);
        }
        
        // Get the argument from command line
        String input = args[0];
        
        // Print the argument
        System.out.println("Original: " + input);
        
        // Trim trailing whitespace
        input = input.replaceAll("\\\\s+$", "");
        
        // Print the trimmed argument
        System.out.println("Trimmed: " + input);
        
        // Exit the program
        System.exit(0);
    }
}
