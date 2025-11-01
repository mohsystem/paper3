
public class Lse7 {
    public static void processArguments(String[] args) {
        // Validate we have at least 2 arguments
        if (args == null || args.length < 2) {
            System.out.println("Error: Insufficient arguments");
            return;
        }
        
        try {
            // Convert string at index 1 to integer
            int index = Integer.parseInt(args[1]);
            
            // Validate index is within bounds
            if (index < 0 || index >= args.length) {
                System.out.println("Error: Index out of bounds");
                return;
            }
            
            // Print the argument at the specified index
            System.out.println(args[index]);
            
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid index format");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index
        String[] test1 = {"arg0", "2", "Hello", "World"};
        System.out.print("Test 1: ");
        processArguments(test1);
        
        // Test case 2: Index out of bounds
        String[] test2 = {"arg0", "10", "Hello"};
        System.out.print("Test 2: ");
        processArguments(test2);
        
        // Test case 3: Invalid index format
        String[] test3 = {"arg0", "abc", "Hello"};
        System.out.print("Test 3: ");
        processArguments(test3);
        
        // Test case 4: Insufficient arguments
        String[] test4 = {"arg0"};
        System.out.print("Test 4: ");
        processArguments(test4);
        
        // Test case 5: Access first argument
        String[] test5 = {"First", "0", "Second", "Third"};
        System.out.print("Test 5: ");
        processArguments(test5);
    }
}
