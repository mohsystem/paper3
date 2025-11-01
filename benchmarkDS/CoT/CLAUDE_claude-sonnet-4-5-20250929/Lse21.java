
public class Lse21 {
    public static int processValue(String input) {
        try {
            // Validate input is not null or empty
            if (input == null || input.trim().isEmpty()) {
                System.err.println("Error: Input cannot be null or empty");
                return -1;
            }
            
            // Parse the input safely
            long value = Long.parseLong(input.trim());
            
            // Check for overflow when adding 1000
            if (value > Long.MAX_VALUE - 1000) {
                System.err.println("Error: Integer overflow detected");
                return -1;
            }
            
            if (value < Long.MIN_VALUE + 1000) {
                System.err.println("Error: Integer underflow detected");
                return -1;
            }
            
            long result = value + 1000;
            System.out.println(result);
            return 0;
            
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid number format");
            return -1;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"100", "0", "-500", "999999999", "abc"};
        
        System.out.println("Running test cases:");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\\nTest case " + (i + 1) + ": input = " + testCases[i]);
            processValue(testCases[i]);
        }
        
        // Process command line argument if provided
        if (args.length > 0) {
            System.out.println("\\nProcessing command line argument:");
            System.exit(processValue(args[0]));
        }
    }
}
