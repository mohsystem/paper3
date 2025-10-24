
public class Task71 {
    public static Integer convertStringToInteger(String input) {
        try {
            return Integer.parseInt(input.trim());
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid input - cannot convert '" + input + "' to integer");
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"123", "-456", "0", "abc", "12.34"};
        
        System.out.println("Running test cases:");
        for (String testCase : testCases) {
            System.out.print("Input: '" + testCase + "' -> ");
            Integer result = convertStringToInteger(testCase);
            if (result != null) {
                System.out.println("Output: " + result);
            }
        }
        
        // Command line argument processing
        if (args.length > 0) {
            System.out.println("\\nCommand line argument:");
            System.out.print("Input: '" + args[0] + "' -> ");
            Integer result = convertStringToInteger(args[0]);
            if (result != null) {
                System.out.println("Output: " + result);
            }
        } else {
            System.out.println("\\nNo command line arguments provided.");
        }
    }
}
