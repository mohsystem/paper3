public class Task26 {
    public static int findOdd(int[] arr) {
        int result = 0;
        for (int number : arr) {
            result ^= number;
        }
        return result;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {7};
        System.out.println("Test Case 1: [7] -> " + findOdd(arr1));

        // Test Case 2
        int[] arr2 = {0};
        System.out.println("Test Case 2: [0] -> " + findOdd(arr2));

        // Test Case 3
        int[] arr3 = {1, 1, 2};
        System.out.println("Test Case 3: [1, 1, 2] -> " + findOdd(arr3));

        // Test Case 4
        int[] arr4 = {0, 1, 0, 1, 0};
        System.out.println("Test Case 4: [0, 1, 0, 1, 0] -> " + findOdd(arr4));

        // Test Case 5
        int[] arr5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
        System.out.println("Test Case 5: [1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1] -> " + findOdd(arr5));
    }
}