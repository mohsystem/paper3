
public class Task163 {
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        
        if (arr.length == 1) {
            return 1;
        }
        
        int maxLength = 1;
        int currentLength = 1;
        
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] - arr[i - 1] == 1 || arr[i] - arr[i - 1] == -1) {
                currentLength++;
                maxLength = Math.max(maxLength, currentLength);
            } else {
                currentLength = 1;
            }
        }
        
        return maxLength;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 3, 5, 6, 7, 8, 9};
        System.out.println("Test 1: " + longestRun(test1) + " (Expected: 5)");
        
        // Test case 2
        int[] test2 = {1, 2, 3, 10, 11, 15};
        System.out.println("Test 2: " + longestRun(test2) + " (Expected: 3)");
        
        // Test case 3
        int[] test3 = {5, 4, 2, 1};
        System.out.println("Test 3: " + longestRun(test3) + " (Expected: 2)");
        
        // Test case 4
        int[] test4 = {3, 5, 7, 10, 15};
        System.out.println("Test 4: " + longestRun(test4) + " (Expected: 1)");
        
        // Test case 5
        int[] test5 = {10, 9, 8, 7, 6, 5, 4};
        System.out.println("Test 5: " + longestRun(test5) + " (Expected: 7)");
    }
}
