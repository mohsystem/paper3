
public class Lse7 {
    public static void main(String[] args) {
        try {
            // Validate that we have at least 2 arguments
            if (args == null || args.length < 2) {
                System.err.println("Error: Insufficient arguments");
                return;
            }
            
            // Validate the index argument (args[1]) is not null or empty
            if (args[1] == null || args[1].trim().isEmpty()) {
                System.err.println("Error: Invalid index");
                return;
            }
            
            int index;
            try {
                // Convert string to integer with validation
                index = Integer.parseInt(args[1].trim());
            } catch (NumberFormatException e) {
                System.err.println("Error: Invalid number format");
                return;
            }
            
            // Validate index is within valid range [0, args.length-1]
            // Prevent negative index and out of bounds access
            if (index < 0 || index >= args.length) {
                System.err.println("Error: Index out of bounds");
                return;
            }
            
            // Safely print the argument at the validated index
            System.out.println(args[index]);
            
        } catch (Exception e) {
            // Catch any unexpected exceptions, log generically
            System.err.println("Error: Operation failed");
        }
    }
}
