
public class Task192 {
    public static boolean xorGame(int[] nums) {
        // Input validation
        if (nums == null || nums.length == 0) {
            return false;
        }
        
        // Calculate XOR of all elements
        int xor = 0;
        for (int num : nums) {
            xor ^= num;
        }
        
        // Alice wins if XOR is 0 or array length is even
        return xor == 0 || nums.length % 2 == 0;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 1, 2};
        System.out.println("Test 1: " + xorGame(test1)); // Expected: false
        
        // Test case 2
        int[] test2 = {0, 1};
        System.out.println("Test 2: " + xorGame(test2)); // Expected: true
        
        // Test case 3
        int[] test3 = {1, 2, 3};
        System.out.println("Test 3: " + xorGame(test3)); // Expected: true
        
        // Test case 4
        int[] test4 = {1};
        System.out.println("Test 4: " + xorGame(test4)); // Expected: false
        
        // Test case 5
        int[] test5 = {2, 2};
        System.out.println("Test 5: " + xorGame(test5)); // Expected: true
    }
}
