import java.util.Arrays;

public class Task144 {

    /**
     * Merges two sorted integer arrays into a single sorted array.
     *
     * @param arr1 The first sorted array. Can be null, treated as an empty array.
     * @param arr2 The second sorted array. Can be null, treated as an empty array.
     * @return A new array containing all elements from arr1 and arr2 in sorted order.
     */
    public static int[] mergeSortedArrays(int[] arr1, int[] arr2) {
        if (arr1 == null) {
            arr1 = new int[0];
        }
        if (arr2 == null) {
            arr2 = new int[0];
        }

        int n1 = arr1.length;
        int n2 = arr2.length;
        int[] mergedArray = new int[n1 + n2];

        int i = 0, j = 0, k = 0;

        // Traverse both arrays and insert the smaller element into the merged array
        while (i < n1 && j < n2) {
            if (arr1[i] <= arr2[j]) {
                mergedArray[k++] = arr1[i++];
            } else {
                mergedArray[k++] = arr2[j++];
            }
        }

        // Copy remaining elements of arr1, if any
        while (i < n1) {
            mergedArray[k++] = arr1[i++];
        }

        // Copy remaining elements of arr2, if any
        while (j < n2) {
            mergedArray[k++] = arr2[j++];
        }

        return mergedArray;
    }

    public static void main(String[] args) {
        // Test Case 1: General case
        int[] arr1_1 = {1, 3, 5};
        int[] arr1_2 = {2, 4, 6};
        System.out.println("Test Case 1:");
        System.out.println("Input: " + Arrays.toString(arr1_1) + ", " + Arrays.toString(arr1_2));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(arr1_1, arr1_2)));
        System.out.println();

        // Test Case 2: One array is empty
        int[] arr2_1 = {10, 20, 30};
        int[] arr2_2 = {};
        System.out.println("Test Case 2:");
        System.out.println("Input: " + Arrays.toString(arr2_1) + ", " + Arrays.toString(arr2_2));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(arr2_1, arr2_2)));
        System.out.println();

        // Test Case 3: Both arrays are empty
        int[] arr3_1 = {};
        int[] arr3_2 = {};
        System.out.println("Test Case 3:");
        System.out.println("Input: " + Arrays.toString(arr3_1) + ", " + Arrays.toString(arr3_2));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(arr3_1, arr3_2)));
        System.out.println();

        // Test Case 4: Arrays with different lengths
        int[] arr4_1 = {1, 2, 9, 15};
        int[] arr4_2 = {3, 10};
        System.out.println("Test Case 4:");
        System.out.println("Input: " + Arrays.toString(arr4_1) + ", " + Arrays.toString(arr4_2));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(arr4_1, arr4_2)));
        System.out.println();

        // Test Case 5: Arrays with duplicate and negative numbers
        int[] arr5_1 = {-5, 0, 5};
        int[] arr5_2 = {-10, 0, 10};
        System.out.println("Test Case 5:");
        System.out.println("Input: " + Arrays.toString(arr5_1) + ", " + Arrays.toString(arr5_2));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(arr5_1, arr5_2)));
        System.out.println();
    }
}