
public class Lse21 {
    public static int addThousand(int value) {
        int result = value + 1000;
        System.out.println(result);
        return 0;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test case 1:");
        addThousand(5);
        
        System.out.println("\\nTest case 2:");
        addThousand(100);
        
        System.out.println("\\nTest case 3:");
        addThousand(-50);
        
        System.out.println("\\nTest case 4:");
        addThousand(0);
        
        System.out.println("\\nTest case 5:");
        addThousand(9999);
        
        // If command line argument is provided
        if (args.length > 0) {
            try {
                int value = Integer.parseInt(args[0]);
                System.out.println("\\nCommand line input:");
                addThousand(value);
            } catch (NumberFormatException e) {
                System.out.println("Invalid input");
            }
        }
    }
}
