
public class Task146 {
    public static int findMissingNumber(int[] arr, int n) {
        // Calculate expected sum of 1 to n
        int expectedSum = n * (n + 1) / 2;
        
        // Calculate actual sum of array elements
        int actualSum = 0;
        for (int num : arr) {
            actualSum += num;
        }
        
        // Missing number is the difference
        return expectedSum - actualSum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Missing number is 5
        int[] test1 = {1, 2, 3, 4, 6, 7, 8, 9, 10};
        System.out.println("Test 1 - Missing number: " + findMissingNumber(test1, 10));
        
        // Test case 2: Missing number is 1
        int[] test2 = {2, 3, 4, 5};
        System.out.println("Test 2 - Missing number: " + findMissingNumber(test2, 5));
        
        // Test case 3: Missing number is 10
        int[] test3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        System.out.println("Test 3 - Missing number: " + findMissingNumber(test3, 10));
        
        // Test case 4: Missing number is 3
        int[] test4 = {1, 2, 4, 5, 6};
        System.out.println("Test 4 - Missing number: " + findMissingNumber(test4, 6));
        
        // Test case 5: Missing number is 7
        int[] test5 = {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12};
        System.out.println("Test 5 - Missing number: " + findMissingNumber(test5, 12));
    }
}
