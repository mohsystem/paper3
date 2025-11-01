
public class Lse19 {
    public static int addThousand(String input) {
        if (input == null || input.trim().isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
        }
        
        try {
            long value = Long.parseLong(input.trim());
            long result = value + 1000;
            
            // Check for overflow
            if (result > Integer.MAX_VALUE || result < Integer.MIN_VALUE) {
                throw new ArithmeticException("Result exceeds integer bounds");
            }
            
            return (int) result;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid number format: " + input);
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"100", "500", "-200", "0", "2147482647"};
        
        for (String testCase : testCases) {
            try {
                int result = addThousand(testCase);
                System.out.println("Input: " + testCase + " -> Output: " + result);
            } catch (Exception e) {
                System.out.println("Input: " + testCase + " -> Error: " + e.getMessage());
            }
        }
        
        // Command line argument processing
        if (args.length > 0) {
            try {
                int result = addThousand(args[0]);
                System.out.println("\\nCommand line input: " + args[0] + " -> Output: " + result);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }
        }
    }
}
