
public class Lse19 {
    public static int addThousand(int value) {
        return value + 1000;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + addThousand(500));      // Expected: 1500
        System.out.println("Test 2: " + addThousand(0));        // Expected: 1000
        System.out.println("Test 3: " + addThousand(-100));     // Expected: 900
        System.out.println("Test 4: " + addThousand(2500));     // Expected: 3500
        System.out.println("Test 5: " + addThousand(-1000));    // Expected: 0
        
        // Command line argument handling
        if (args.length > 0) {
            try {
                int value = Integer.parseInt(args[0]);
                System.out.println("Command line result: " + addThousand(value));
            } catch (NumberFormatException e) {
                System.err.println("Invalid input. Please provide a valid integer.");
            }
        }
    }
}
