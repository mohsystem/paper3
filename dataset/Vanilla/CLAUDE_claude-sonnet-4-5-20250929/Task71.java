
public class Task71 {
    public static Integer convertToInteger(String input) {
        try {
            return Integer.parseInt(input.trim());
        } catch (NumberFormatException e) {
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"123", "-456", "0", "abc", "12.34"};
        
        if (args.length > 0) {
            Integer result = convertToInteger(args[0]);
            if (result != null) {
                System.out.println("Converted integer: " + result);
            } else {
                System.out.println("Invalid integer input: " + args[0]);
            }
        } else {
            System.out.println("Running test cases:");
            for (String testCase : testCases) {
                Integer result = convertToInteger(testCase);
                if (result != null) {
                    System.out.println("Input: '" + testCase + "' -> Output: " + result);
                } else {
                    System.out.println("Input: '" + testCase + "' -> Output: Invalid (cannot convert)");
                }
            }
        }
    }
}
