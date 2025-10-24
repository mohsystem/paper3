
public class Task193 {
    public static boolean xorGame(int[] nums) {
        if (nums == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        if (nums.length < 1 || nums.length > 1000) {
            throw new IllegalArgumentException("Array length must be between 1 and 1000");
        }
        
        int xorSum = 0;
        for (int num : nums) {
            if (num < 0 || num >= 65536) {
                throw new IllegalArgumentException("Array elements must be between 0 and 65535");
            }
            xorSum ^= num;
        }
        
        return xorSum == 0 || nums.length % 2 == 0;
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
        int[] test5 = {2, 2, 2, 2};
        System.out.println("Test 5: " + xorGame(test5)); // Expected: true
    }
}
