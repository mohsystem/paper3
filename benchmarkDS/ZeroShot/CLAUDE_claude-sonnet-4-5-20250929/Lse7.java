
public class Lse7 {
    public static void processCommandLineArguments(String[] args, int index) {
        if (args == null || args.length == 0) {
            System.out.println("No arguments provided");
            return;
        }
        
        if (index < 0 || index >= args.length) {
            System.out.println("Index out of bounds");
            return;
        }
        
        System.out.println(args[index]);
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal case
        String[] test1 = {"arg0", "2", "arg2", "arg3"};
        System.out.println("Test 1:");
        int index1 = Integer.parseInt(test1[1]);
        processCommandLineArguments(test1, index1);
        
        // Test case 2: Index at boundary
        String[] test2 = {"arg0", "0", "arg2"};
        System.out.println("\\nTest 2:");
        int index2 = Integer.parseInt(test2[1]);
        processCommandLineArguments(test2, index2);
        
        // Test case 3: Last element
        String[] test3 = {"arg0", "3", "arg2", "arg3"};
        System.out.println("\\nTest 3:");
        int index3 = Integer.parseInt(test3[1]);
        processCommandLineArguments(test3, index3);
        
        // Test case 4: Out of bounds
        String[] test4 = {"arg0", "5", "arg2"};
        System.out.println("\\nTest 4:");
        int index4 = Integer.parseInt(test4[1]);
        processCommandLineArguments(test4, index4);
        
        // Test case 5: Empty array handling
        String[] test5 = {};
        System.out.println("\\nTest 5:");
        processCommandLineArguments(test5, 0);
    }
}
