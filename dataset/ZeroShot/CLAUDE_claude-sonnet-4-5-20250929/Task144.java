
public class Task144 {
    public static int[] mergeSortedArrays(int[] arr1, int[] arr2) {
        if (arr1 == null) arr1 = new int[0];
        if (arr2 == null) arr2 = new int[0];
        
        int[] result = new int[arr1.length + arr2.length];
        int i = 0, j = 0, k = 0;
        
        while (i < arr1.length && j < arr2.length) {
            if (arr1[i] <= arr2[j]) {
                result[k++] = arr1[i++];
            } else {
                result[k++] = arr2[j++];
            }
        }
        
        while (i < arr1.length) {
            result[k++] = arr1[i++];
        }
        
        while (j < arr2.length) {
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
        
        // Test case 3: Different lengths
        int[] arr5 = {1, 5, 9};
        int[] arr6 = {2, 3, 4, 6, 7, 8};
        int[] result3 = mergeSortedArrays(arr5, arr6);
        System.out.print("Test 3: ");
        for (int num : result3) System.out.print(num + " ");
        System.out.println();
        
        // Test case 4: Duplicate elements
        int[] arr7 = {1, 3, 5, 5};
        int[] arr8 = {2, 3, 5, 6};
        int[] result4 = mergeSortedArrays(arr7, arr8);
        System.out.print("Test 4: ");
        for (int num : result4) System.out.print(num + " ");
        System.out.println();
        
        // Test case 5: Negative numbers
        int[] arr9 = {-5, -3, 0, 2};
        int[] arr10 = {-4, -1, 1, 3};
        int[] result5 = mergeSortedArrays(arr9, arr10);
        System.out.print("Test 5: ");
        for (int num : result5) System.out.print(num + " ");
        System.out.println();
    }
}
