
public class Lse21 {
    public static int addThousand(int value) {
        return value + 1000;
    }
    
    public static void main(String[] args) {
        // Test cases
        int[] testCases = {100, -500, 0, 9999, -1000};
        
        for (int testCase : testCases) {
            int result = addThousand(testCase);
            System.out.println("Input: " + testCase + ", Output: " + result);
        }
        
        // Command line argument handling
        if (args.length > 0) {
            try {
                int value = Integer.parseInt(args[0]);
                int result = addThousand(value);
                System.out.println(result);
            } catch (NumberFormatException e) {
                System.err.println("Invalid input. Please provide a valid integer.");
                System.exit(1);
            }
        }
        
        System.exit(0);
    }
}
