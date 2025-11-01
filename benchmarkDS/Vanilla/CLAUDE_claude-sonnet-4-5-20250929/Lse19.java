
public class Lse19 {
    public static int addThousand(int value) {
        return value + 1000;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + addThousand(5) + " (Expected: 1005)");
        System.out.println("Test 2: " + addThousand(0) + " (Expected: 1000)");
        System.out.println("Test 3: " + addThousand(-500) + " (Expected: 500)");
        System.out.println("Test 4: " + addThousand(2000) + " (Expected: 3000)");
        System.out.println("Test 5: " + addThousand(-1000) + " (Expected: 0)");
        
        // Command line argument handling
        if (args.length > 0) {
            try {
                int value = Integer.parseInt(args[0]);
                System.out.println("Command line input: " + value + " -> Result: " + addThousand(value));
            } catch (NumberFormatException e) {
                System.out.println("Invalid input. Please provide a valid integer.");
            }
        }
    }
}
