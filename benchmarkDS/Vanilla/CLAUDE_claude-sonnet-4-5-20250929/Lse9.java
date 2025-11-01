
public class Lse9 {
    public static void printArgumentAtIndex(String[] args, int index) {
        if (index >= 0 && index < args.length) {
            System.out.println(args[index]);
        } else {
            System.out.println("Index out of bounds");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Index 0
        String[] test1 = {"first", "second", "third"};
        System.out.print("Test 1 (index 0): ");
        printArgumentAtIndex(test1, 0);
        
        // Test case 2: Index 1
        String[] test2 = {"apple", "banana", "cherry"};
        System.out.print("Test 2 (index 1): ");
        printArgumentAtIndex(test2, 1);
        
        // Test case 3: Index 2
        String[] test3 = {"hello", "world", "java", "programming"};
        System.out.print("Test 3 (index 2): ");
        printArgumentAtIndex(test3, 2);
        
        // Test case 4: Index out of bounds
        String[] test4 = {"one", "two"};
        System.out.print("Test 4 (index 5): ");
        printArgumentAtIndex(test4, 5);
        
        // Test case 5: Index 3
        String[] test5 = {"a", "b", "c", "d", "e"};
        System.out.print("Test 5 (index 3): ");
        printArgumentAtIndex(test5, 3);
    }
}
