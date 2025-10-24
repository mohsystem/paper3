
public class Task27 {
    public static int findOutlier(int[] array) {
        // Security: Validate input to prevent null pointer exceptions
        if (array == null || array.length < 3) {
            throw new IllegalArgumentException("Array must not be null and must have at least 3 elements");
        }
        
        // Count even and odd numbers in first 3 elements to determine the pattern
        int evenCount = 0;
        int oddCount = 0;
        
        // Check first 3 elements to determine if we're looking for even or odd outlier\n        for (int i = 0; i < 3; i++) {\n            if (array[i] % 2 == 0) {\n                evenCount++;\n            } else {\n                oddCount++;\n            }\n        }\n        \n        // Determine if we're looking for the odd or even outlier
        boolean lookingForOdd = evenCount > oddCount;
        
        // Find and return the outlier
        for (int num : array) {
            if (lookingForOdd && num % 2 != 0) {
                return num;
            } else if (!lookingForOdd && num % 2 == 0) {
                return num;
            }
        }
        
        // This should never be reached given valid input
        throw new IllegalStateException("No outlier found in array");
    }
    
    public static void main(String[] args) {
        // Test case 1: Only odd number
        int[] test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        System.out.println("Test 1: " + findOutlier(test1)); // Expected: 11
        
        // Test case 2: Only even number
        int[] test2 = {160, 3, 1719, 19, 11, 13, -21};
        System.out.println("Test 2: " + findOutlier(test2)); // Expected: 160
        
        // Test case 3: Negative odd outlier
        int[] test3 = {2, 4, 6, 8, 10, -1};
        System.out.println("Test 3: " + findOutlier(test3)); // Expected: -1
        
        // Test case 4: Negative even outlier
        int[] test4 = {1, 3, 5, 7, -2, 9};
        System.out.println("Test 4: " + findOutlier(test4)); // Expected: -2
        
        // Test case 5: Zero as outlier
        int[] test5 = {1, 3, 5, 0, 7, 9};
        System.out.println("Test 5: " + findOutlier(test5)); // Expected: 0
    }
}
