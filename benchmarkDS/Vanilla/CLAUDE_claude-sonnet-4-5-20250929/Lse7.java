
public class Lse7 {
    public static void printArgumentAtIndex(String[] args, String indexStr) {
        int index = Integer.parseInt(indexStr);
        if (index >= 0 && index < args.length) {
            System.out.println(args[index]);
        } else {
            System.out.println("Index out of bounds");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] test1 = {"program", "2", "hello", "world", "java"};
        System.out.print("Test 1: ");
        printArgumentAtIndex(test1, test1[1]);
        
        // Test case 2
        String[] test2 = {"program", "0", "first", "second", "third"};
        System.out.print("Test 2: ");
        printArgumentAtIndex(test2, test2[1]);
        
        // Test case 3
        String[] test3 = {"program", "3", "alpha", "beta", "gamma", "delta"};
        System.out.print("Test 3: ");
        printArgumentAtIndex(test3, test3[1]);
        
        // Test case 4
        String[] test4 = {"program", "1", "testing"};
        System.out.print("Test 4: ");
        printArgumentAtIndex(test4, test4[1]);
        
        // Test case 5
        String[] test5 = {"program", "5", "one", "two", "three"};
        System.out.print("Test 5: ");
        printArgumentAtIndex(test5, test5[1]);
    }
}
