import java.util.Arrays;

public class Task144 {

    /**
     * Merges two sorted integer arrays into a single sorted array.
     *
     * @param arr1 The first sorted integer array.
     * @param arr2 The second sorted integer array.
     * @return A new sorted array containing all elements from arr1 and arr2.
     */
    public static int[] mergeArrays(int[] arr1, int[] arr2) {
        // Handle null or empty array edge cases
        if (arr1 == null || arr1.length == 0) {
            return (arr2 == null) ? new int[0] : arr2.clone();
        }
        if (arr2 == null || arr2.length == 0) {
            return arr1.clone();
        }

        int n1 = arr1.length;
        int n2 = arr2.length;
        int[] mergedArray = new int[n1 + n2];

        int i = 0; // Pointer for arr1
        int j = 0; // Pointer for arr2
        int k = 0; // Pointer for mergedArray

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
        // Test Case 1: Standard merge
        int[] arr1 = {1, 3, 5, 7};
        int[] arr2 = {2, 4, 6, 8};
        System.out.println("Test Case 1:");
        System.out.println("Array 1: " + Arrays.toString(arr1));
        System.out.println("Array 2: " + Arrays.toString(arr2));
        System.out.println("Merged: " + Arrays.toString(mergeArrays(arr1, arr2)));
        System.out.println();

        // Test Case 2: Arrays with different lengths
        int[] arr3 = {10, 20, 30};
        int[] arr4 = {5, 15, 25, 35, 45};
        System.out.println("Test Case 2:");
        System.out.println("Array 1: " + Arrays.toString(arr3));
        System.out.println("Array 2: " + Arrays.toString(arr4));
        System.out.println("Merged: " + Arrays.toString(mergeArrays(arr3, arr4)));
        System.out.println();

        // Test Case 3: One array is empty
        int[] arr5 = {1, 2, 3};
        int[] arr6 = {};
        System.out.println("Test Case 3:");
        System.out.println("Array 1: " + Arrays.toString(arr5));
        System.out.println("Array 2: " + Arrays.toString(arr6));
        System.out.println("Merged: " + Arrays.toString(mergeArrays(arr5, arr6)));
        System.out.println();

        // Test Case 4: Both arrays are empty
        int[] arr7 = {};
        int[] arr8 = {};
        System.out.println("Test Case 4:");
        System.out.println("Array 1: " + Arrays.toString(arr7));
        System.out.println("Array 2: " + Arrays.toString(arr8));
        System.out.println("Merged: " + Arrays.toString(mergeArrays(arr7, arr8)));
        System.out.println();

        // Test Case 5: Arrays with negative numbers and duplicates
        int[] arr9 = {-5, 0, 5, 5, 15};
        int[] arr10 = {-10, -5, 10, 20};
        System.out.println("Test Case 5:");
        System.out.println("Array 1: " + Arrays.toString(arr9));
        System.out.println("Array 2: " + Arrays.toString(arr10));
        System.out.println("Merged: " + Arrays.toString(mergeArrays(arr9, arr10)));
        System.out.println();
    }
}