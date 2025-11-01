
public class Lse8 {
    public static String getValueAtIndex(String[] args, int index) {
        if (args == null || index < 0 || index >= args.length) {
            return null;
        }
        return args[index];
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] test1 = {"program", "1", "hello", "world"};
        int index1 = Integer.parseInt(test1[1]);
        System.out.println("Test 1: " + getValueAtIndex(test1, index1));
        
        // Test case 2
        String[] test2 = {"program", "2", "apple", "banana", "cherry"};
        int index2 = Integer.parseInt(test2[1]);
        System.out.println("Test 2: " + getValueAtIndex(test2, index2));
        
        // Test case 3
        String[] test3 = {"program", "0", "first"};
        int index3 = Integer.parseInt(test3[1]);
        System.out.println("Test 3: " + getValueAtIndex(test3, index3));
        
        // Test case 4
        String[] test4 = {"program", "3", "a", "b", "c", "d"};
        int index4 = Integer.parseInt(test4[1]);
        System.out.println("Test 4: " + getValueAtIndex(test4, index4));
        
        // Test case 5
        String[] test5 = {"program", "1", "test"};
        int index5 = Integer.parseInt(test5[1]);
        System.out.println("Test 5: " + getValueAtIndex(test5, index5));
    }
}
