
public class Task27 {
    public static int findOutlier(int[] integers) {
        // Validate input
        if (integers == null || integers.length < 3) {
            throw new IllegalArgumentException("Array must have at least 3 elements");
        }
        
        // Count even and odd numbers in first 3 elements to determine majority
        int evenCount = 0;
        int oddCount = 0;
        
        for (int i = 0; i < Math.min(3, integers.length); i++) {
            if (integers[i] % 2 == 0) {
                evenCount++;
            } else {
                oddCount++;
            }
        }
        
        // Determine if we're looking for even or odd outlier
        boolean lookingForEven = oddCount > evenCount;
        
        // Find and return the outlier
        for (int num : integers) {
            if (lookingForEven && num % 2 == 0) {
                return num;
            } else if (!lookingForEven && num % 2 != 0) {
                return num;
            }
        }
        
        throw new IllegalArgumentException("No outlier found");
    }
    
    public static void main(String[] args) {
        // Test case 1: outlier is odd
        int[] test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        System.out.println("Test 1: " + findOutlier(test1)); // Expected: 11
        
        // Test case 2: outlier is even
        int[] test2 = {160, 3, 1719, 19, 11, 13, -21};
        System.out.println("Test 2: " + findOutlier(test2)); // Expected: 160
        
        // Test case 3: outlier is odd with negative numbers
        int[] test3 = {2, 6, 8, -10, 3};
        System.out.println("Test 3: " + findOutlier(test3)); // Expected: 3
        
        // Test case 4: outlier is even with negative numbers
        int[] test4 = {1, 1, 0, 1, 1};
        System.out.println("Test 4: " + findOutlier(test4)); // Expected: 0
        
        // Test case 5: larger array
        int[] test5 = {1, 3, 5, 7, 9, 11, 13, 15, 2};
        System.out.println("Test 5: " + findOutlier(test5)); // Expected: 2
    }
}
