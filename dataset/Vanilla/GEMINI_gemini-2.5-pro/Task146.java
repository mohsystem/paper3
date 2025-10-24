public class Task146 {

    public static int findMissingNumber(int[] arr) {
        // The array is supposed to have numbers from 1 to n, but has n-1 elements.
        // So, n is the length of the array + 1.
        int n = arr.length + 1;

        // Calculate the expected sum of the first n natural numbers.
        // Using long to prevent potential integer overflow for very large n.
        long expectedSum = (long)n * (n + 1) / 2;

        // Calculate the actual sum of elements in the array.
        long actualSum = 0;
        for (int num : arr) {
            actualSum += num;
        }

        // The difference is the missing number.
        return (int)(expectedSum - actualSum);
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {1, 2, 4, 5};
        System.out.println("Test Case 1: Missing number in [1, 2, 4, 5] is " + findMissingNumber(arr1));

        // Test Case 2
        int[] arr2 = {2, 3, 1, 5};
        System.out.println("Test Case 2: Missing number in [2, 3, 1, 5] is " + findMissingNumber(arr2));

        // Test Case 3
        int[] arr3 = {1};
        System.out.println("Test Case 3: Missing number in [1] is " + findMissingNumber(arr3));

        // Test Case 4
        int[] arr4 = {1, 2, 3, 4, 5, 6, 8, 9};
        System.out.println("Test Case 4: Missing number in [1, 2, 3, 4, 5, 6, 8, 9] is " + findMissingNumber(arr4));
        
        // Test Case 5
        int[] arr5 = {2};
        System.out.println("Test Case 5: Missing number in [2] is " + findMissingNumber(arr5));
    }
}