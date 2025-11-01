
public class Lse20 {
    private static final int OFFSET = 1000;
    
    public static int addOffset(String input) {
        try {
            int value = Integer.parseInt(input);
            // Check for potential integer overflow
            if (value > Integer.MAX_VALUE - OFFSET) {
                throw new ArithmeticException("Integer overflow detected");
            }
            return value + OFFSET;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid input: not a valid integer");
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"100", "0", "-500", "2147482647", "50"};
        
        for (String testCase : testCases) {
            try {
                int result = addOffset(testCase);
                System.out.println("Input: " + testCase + " -> Output: " + result);
            } catch (Exception e) {
                System.out.println("Input: " + testCase + " -> Error: " + e.getMessage());
            }
        }
    }
}
