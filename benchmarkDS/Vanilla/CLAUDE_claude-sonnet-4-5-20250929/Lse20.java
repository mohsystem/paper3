
public class Lse20 {
    private static final int OFFSET = 1000;
    
    public static int addOffset(int value) {
        return value + OFFSET;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: Input = 5, Output = " + addOffset(5));
        
        // Test case 2
        System.out.println("Test 2: Input = 100, Output = " + addOffset(100));
        
        // Test case 3
        System.out.println("Test 3: Input = -500, Output = " + addOffset(-500));
        
        // Test case 4
        System.out.println("Test 4: Input = 0, Output = " + addOffset(0));
        
        // Test case 5
        System.out.println("Test 5: Input = 2500, Output = " + addOffset(2500));
        
        // If command line argument is provided
        if (args.length > 0) {
            try {
                int inputValue = Integer.parseInt(args[0]);
                int result = addOffset(inputValue);
                System.out.println("Command line input: " + inputValue + ", Result: " + result);
            } catch (NumberFormatException e) {
                System.out.println("Invalid input. Please provide a valid integer.");
            }
        }
    }
}
