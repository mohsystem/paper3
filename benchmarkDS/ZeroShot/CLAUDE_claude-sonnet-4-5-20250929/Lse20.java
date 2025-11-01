
public class Lse20 {
    private static final int OFFSET = 1000;
    
    public static int addOffset(int value) {
        return value + OFFSET;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: " + addOffset(100) + " (Expected: 1100)");
        System.out.println("Test Case 2: " + addOffset(0) + " (Expected: 1000)");
        System.out.println("Test Case 3: " + addOffset(-500) + " (Expected: 500)");
        System.out.println("Test Case 4: " + addOffset(2500) + " (Expected: 3500)");
        System.out.println("Test Case 5: " + addOffset(-1000) + " (Expected: 0)");
        
        // Read from command line if argument provided
        if (args.length > 0) {
            try {
                int inputValue = Integer.parseInt(args[0]);
                int result = addOffset(inputValue);
                System.out.println("\\nCommand line input: " + inputValue);
                System.out.println("Result after adding offset: " + result);
            } catch (NumberFormatException e) {
                System.err.println("Error: Invalid number format");
            }
        }
    }
}
