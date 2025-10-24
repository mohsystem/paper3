import java.util.Arrays;

public class Task144 {

    /**
     * Merges two sorted integer arrays into a single sorted array.
     *
     * @param arr1 The first sorted integer array.
     * @param arr2 The second sorted integer array.
     * @return A new array containing all elements from arr1 and arr2 in sorted order.
     */
    public static int[] mergeArrays(int[] arr1, int[] arr2) {
        int n1 = arr1.length;
        int n2 = arr2.length;
        int[] result = new int[n1 + n2];

        int i = 0, j = 0, k = 0;

        // Traverse both arrays and insert the smaller element into the result
        while (i < n1 && j < n2) {
            if (arr1[i] <= arr2[j]) {
                result[k++] = arr1[i++];
            } else {
                result[k++] = arr2[j++];
            }
        }

        // Copy remaining elements of arr1, if any
        while (i < n1) {
            result[k++] = arr1[i++];
        }

        // Copy remaining elements of arr2, if any
        while (j < n2) {
            result[k++] = arr2[j++];
        }

        return result;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1_1 = {1, 3, 5};
        int[] arr1_2 = {2, 4, 6};
        System.out.println("Test Case 1: " + Arrays.toString(mergeArrays(arr1_1, arr1_2)));

        // Test Case 2
        int[] arr2_1 = {10, 20, 30};
        int[] arr2_2 = {};
        System.out.println("Test Case 2: " + Arrays.toString(mergeArrays(arr2_1, arr2_2)));

        // Test Case 3
        int[] arr3_1 = {};
        int[] arr3_2 = {5, 15, 25};
        System.out.println("Test Case 3: " + Arrays.toString(mergeArrays(arr3_1, arr3_2)));

        // Test Case 4
        int[] arr4_1 = {1, 5, 9, 10};
        int[] arr4_2 = {2, 3, 8};
        System.out.println("Test Case 4: " + Arrays.toString(mergeArrays(arr4_1, arr4_2)));

        // Test Case 5
        int[] arr5_1 = {1, 2, 3};
        int[] arr5_2 = {4, 5, 6};
        System.out.println("Test Case 5: " + Arrays.toString(mergeArrays(arr5_1, arr5_2)));
    }
}