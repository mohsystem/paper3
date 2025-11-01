
public class Lse8 {
    public static String getValueAtIndex(String[] args, int index) {
        if (args == null || index < 0 || index >= args.length) {
            return null;
        }
        return args[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal case with valid index
        String[] test1 = {"arg0", "arg1", "arg2", "arg3"};
        System.out.println("Test 1: " + getValueAtIndex(test1, 1));
        
        // Test case 2: Index 0
        String[] test2 = {"first", "second", "third"};
        System.out.println("Test 2: " + getValueAtIndex(test2, 0));
        
        // Test case 3: Last index
        String[] test3 = {"a", "b", "c", "d", "e"};
        System.out.println("Test 3: " + getValueAtIndex(test3, 4));
        
        // Test case 4: Out of bounds index
        String[] test4 = {"x", "y", "z"};
        System.out.println("Test 4: " + getValueAtIndex(test4, 5));
        
        // Test case 5: Negative index
        String[] test5 = {"hello", "world"};
        System.out.println("Test 5: " + getValueAtIndex(test5, -1));
    }
}
