
public class Task9 {
    public static String oddOrEven(int[] array) {
        if (array == null || array.length == 0) {
            return "even";
        }
        
        int sum = 0;
        for (int num : array) {
            sum += num;
        }
        
        return (sum % 2 == 0) ? "even" : "odd";
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {0};
        System.out.println("Test 1: " + oddOrEven(test1));
        
        // Test case 2
        int[] test2 = {0, 1, 4};
        System.out.println("Test 2: " + oddOrEven(test2));
        
        // Test case 3
        int[] test3 = {0, -1, -5};
        System.out.println("Test 3: " + oddOrEven(test3));
        
        // Test case 4
        int[] test4 = {};
        System.out.println("Test 4: " + oddOrEven(test4));
        
        // Test case 5
        int[] test5 = {2, 4, 6, 8};
        System.out.println("Test 5: " + oddOrEven(test5));
    }
}
