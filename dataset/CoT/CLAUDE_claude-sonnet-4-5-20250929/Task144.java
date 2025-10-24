
public class Task144 {
    public static int[] mergeSortedArrays(int[] arr1, int[] arr2) {
        // Input validation
        if (arr1 == null) arr1 = new int[0];
        if (arr2 == null) arr2 = new int[0];
        
        int len1 = arr1.length;
        int len2 = arr2.length;
        int[] result = new int[len1 + len2];
        
        int i = 0, j = 0, k = 0;
        
        // Merge arrays while both have elements
        while (i < len1 && j < len2) {
            if (arr1[i] <= arr2[j]) {
                result[k++] = arr1[i++];
            } else {
                result[k++] = arr2[j++];
            }
        }
        
        // Copy remaining elements from arr1
        while (i < len1) {
            result[k++] = arr1[i++];
        }
        
        // Copy remaining elements from arr2
        while (j < len2) {
            result[k++] = arr2[j++];
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal case
        int[] arr1 = {1, 3, 5, 7};
        int[] arr2 = {2, 4, 6, 8};
        int[] result1 = mergeSortedArrays(arr1, arr2);
        System.out.print("Test 1: ");
        for (int num : result1) System.out.print(num + " ");
        System.out.println();
        
        // Test case 2: One empty array
        int[] arr3 = {};
        int[] arr4 = {1, 2, 3};
        int[] result2 = mergeSortedArrays(arr3, arr4);
        System.out.print("Test 2: ");
        for (int num : result2) System.out.print(num + " ");
        System.out.println();
        
        // Test case 3: Arrays with duplicates
        int[] arr5 = {1, 3, 3, 5};
        int[] arr6 = {2, 3, 4};
        int[] result3 = mergeSortedArrays(arr5, arr6);
        System.out.print("Test 3: ");
        for (int num : result3) System.out.print(num + " ");
        System.out.println();
        
        // Test case 4: Different lengths
        int[] arr7 = {1, 2};
        int[] arr8 = {3, 4, 5, 6, 7};
        int[] result4 = mergeSortedArrays(arr7, arr8);
        System.out.print("Test 4: ");
        for (int num : result4) System.out.print(num + " ");
        System.out.println();
        
        // Test case 5: Negative numbers
        int[] arr9 = {-5, -3, -1};
        int[] arr10 = {-4, -2, 0, 2};
        int[] result5 = mergeSortedArrays(arr9, arr10);
        System.out.print("Test 5: ");
        for (int num : result5) System.out.print(num + " ");
        System.out.println();
    }
}
