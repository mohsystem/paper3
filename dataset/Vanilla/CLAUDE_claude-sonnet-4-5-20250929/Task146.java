
public class Task146 {
    public static int findMissingNumber(int[] arr, int n) {
        int expectedSum = n * (n + 1) / 2;
        int actualSum = 0;
        for (int num : arr) {
            actualSum += num;
        }
        return expectedSum - actualSum;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 4, 5, 6};
        System.out.println("Test 1 - Missing number: " + findMissingNumber(test1, 6));
        
        // Test case 2
        int[] test2 = {1, 2, 3, 4, 5, 6, 7, 9, 10};
        System.out.println("Test 2 - Missing number: " + findMissingNumber(test2, 10));
        
        // Test case 3
        int[] test3 = {2, 3, 4, 5};
        System.out.println("Test 3 - Missing number: " + findMissingNumber(test3, 5));
        
        // Test case 4
        int[] test4 = {1};
        System.out.println("Test 4 - Missing number: " + findMissingNumber(test4, 2));
        
        // Test case 5
        int[] test5 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16};
        System.out.println("Test 5 - Missing number: " + findMissingNumber(test5, 16));
    }
}
