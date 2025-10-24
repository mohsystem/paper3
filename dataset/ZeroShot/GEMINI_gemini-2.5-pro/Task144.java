import java.util.Arrays;

public class Task144 {

    /**
     * Merges two sorted integer arrays into a single sorted array.
     * This method is secure against null inputs and integer overflow for the combined array size.
     *
     * @param arr1 The first sorted integer array. Can be null.
     * @param arr2 The second sorted integer array. Can be null.
     * @return A new sorted array containing all elements from arr1 and arr2.
     * @throws OutOfMemoryError if the combined size of the arrays exceeds Integer.MAX_VALUE.
     */
    public static int[] mergeSortedArrays(int[] arr1, int[] arr2) {
        // Handle null or empty array inputs
        if (arr1 == null || arr1.length == 0) {
            return (arr2 == null) ? new int[0] : Arrays.copyOf(arr2, arr2.length);
        }
        if (arr2 == null || arr2.length == 0) {
            // arr1 is guaranteed to be non-null and non-empty here
            return Arrays.copyOf(arr1, arr1.length);
        }

        long totalLength = (long) arr1.length + arr2.length;
        // Security: Check for potential integer overflow for array size before allocation
        if (totalLength > Integer.MAX_VALUE) {
            throw new OutOfMemoryError("Combined array size exceeds Integer.MAX_VALUE");
        }

        int[] merged = new int[(int) totalLength];
        int i = 0, j = 0, k = 0;

        // Traverse both arrays and insert the smaller element into the merged array
        while (i < arr1.length && j < arr2.length) {
            if (arr1[i] <= arr2[j]) {
                merged[k++] = arr1[i++];
            } else {
                merged[k++] = arr2[j++];
            }
        }

        // Copy remaining elements from arr1, if any
        while (i < arr1.length) {
            merged[k++] = arr1[i++];
        }

        // Copy remaining elements from arr2, if any
        while (j < arr2.length) {
            merged[k++] = arr2[j++];
        }

        return merged;
    }

    public static void main(String[] args) {
        // Test Case 1: Standard merge
        int[] arr1_1 = {1, 3, 5, 7};
        int[] arr1_2 = {2, 4, 6, 8};
        System.out.println("Test Case 1:");
        System.out.println("Array 1: " + Arrays.toString(arr1_1));
        System.out.println("Array 2: " + Arrays.toString(arr1_2));
        System.out.println("Merged:  " + Arrays.toString(mergeSortedArrays(arr1_1, arr1_2)));
        System.out.println();

        // Test Case 2: One array is empty
        int[] arr2_1 = {10, 20, 30};
        int[] arr2_2 = {};
        System.out.println("Test Case 2:");
        System.out.println("Array 1: " + Arrays.toString(arr2_1));
        System.out.println("Array 2: " + Arrays.toString(arr2_2));
        System.out.println("Merged:  " + Arrays.toString(mergeSortedArrays(arr2_1, arr2_2)));
        System.out.println();

        // Test Case 3: First array is null
        int[] arr3_1 = null;
        int[] arr3_2 = {5, 15, 25};
        System.out.println("Test Case 3:");
        System.out.println("Array 1: null");
        System.out.println("Array 2: " + Arrays.toString(arr3_2));
        System.out.println("Merged:  " + Arrays.toString(mergeSortedArrays(arr3_1, arr3_2)));
        System.out.println();
        
        // Test Case 4: Arrays with duplicate values and different lengths
        int[] arr4_1 = {1, 2, 2, 5, 9};
        int[] arr4_2 = {2, 6, 10};
        System.out.println("Test Case 4:");
        System.out.println("Array 1: " + Arrays.toString(arr4_1));
        System.out.println("Array 2: " + Arrays.toString(arr4_2));
        System.out.println("Merged:  " + Arrays.toString(mergeSortedArrays(arr4_1, arr4_2)));
        System.out.println();

        // Test Case 5: Both arrays are empty
        int[] arr5_1 = {};
        int[] arr5_2 = {};
        System.out.println("Test Case 5:");
        System.out.println("Array 1: " + Arrays.toString(arr5_1));
        System.out.println("Array 2: " + Arrays.toString(arr5_2));
        System.out.println("Merged:  " + Arrays.toString(mergeSortedArrays(arr5_1, arr5_2)));
        System.out.println();
    }
}