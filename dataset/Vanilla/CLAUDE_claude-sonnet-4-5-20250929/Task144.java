
import java.util.Arrays;

public class Task144 {
    public static int[] mergeSortedArrays(int[] arr1, int[] arr2) {
        int n1 = arr1.length;
        int n2 = arr2.length;
        int[] result = new int[n1 + n2];
        
        int i = 0, j = 0, k = 0;
        
        while (i < n1 && j < n2) {
            if (arr1[i] <= arr2[j]) {
                result[k++] = arr1[i++];
            } else {
                result[k++] = arr2[j++];
            }
        }
        
        while (i < n1) {
            result[k++] = arr1[i++];
        }
        
        while (j < n2) {
            result[k++] = arr2[j++];
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] arr1_1 = {1, 3, 5, 7};
        int[] arr2_1 = {2, 4, 6, 8};
        System.out.println("Test 1: " + Arrays.toString(mergeSortedArrays(arr1_1, arr2_1)));
        
        // Test case 2
        int[] arr1_2 = {1, 2, 3};
        int[] arr2_2 = {4, 5, 6};
        System.out.println("Test 2: " + Arrays.toString(mergeSortedArrays(arr1_2, arr2_2)));
        
        // Test case 3
        int[] arr1_3 = {1, 5, 9};
        int[] arr2_3 = {2, 3, 4, 6, 7, 8};
        System.out.println("Test 3: " + Arrays.toString(mergeSortedArrays(arr1_3, arr2_3)));
        
        // Test case 4
        int[] arr1_4 = {};
        int[] arr2_4 = {1, 2, 3};
        System.out.println("Test 4: " + Arrays.toString(mergeSortedArrays(arr1_4, arr2_4)));
        
        // Test case 5
        int[] arr1_5 = {-5, -2, 0, 3};
        int[] arr2_5 = {-3, -1, 2, 4, 6};
        System.out.println("Test 5: " + Arrays.toString(mergeSortedArrays(arr1_5, arr2_5)));
    }
}
