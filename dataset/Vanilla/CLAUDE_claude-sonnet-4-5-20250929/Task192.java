
public class Task192 {
    public static boolean xorGame(int[] nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        return xorSum == 0 || nums.length % 2 == 0;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(xorGame(new int[]{1, 1, 2})); // false
        
        // Test case 2
        System.out.println(xorGame(new int[]{0, 1})); // true
        
        // Test case 3
        System.out.println(xorGame(new int[]{1, 2, 3})); // true
        
        // Test case 4
        System.out.println(xorGame(new int[]{1, 1, 1, 1})); // true
        
        // Test case 5
        System.out.println(xorGame(new int[]{5, 3, 6})); // true
    }
}
