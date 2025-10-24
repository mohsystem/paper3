
import java.util.Arrays;
import java.util.Scanner;

public class Task144 {
    private static final int MAX_ARRAY_SIZE = 100000;
    
    public static int[] mergeSortedArrays(int[] arr1, int[] arr2) {
        if (arr1 == null || arr2 == null) {
            throw new IllegalArgumentException("Input arrays must not be null");
        }
        
        if (arr1.length > MAX_ARRAY_SIZE || arr2.length > MAX_ARRAY_SIZE) {
            throw new IllegalArgumentException("Array size exceeds maximum allowed size");
        }
        
        long totalSize = (long) arr1.length + (long) arr2.length;
        if (totalSize > MAX_ARRAY_SIZE) {
            throw new IllegalArgumentException("Combined array size exceeds maximum allowed size");
        }
        
        int[] result = new int[(int) totalSize];
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
        int[][] testCase1Arr1 = {{1, 3, 5, 7}, {2, 4, 6, 8}};
        int[][] testCase2Arr1 = {{1, 2, 3}, {4, 5, 6}};
        int[][] testCase3Arr1 = {{}, {1, 2, 3}};
        int[][] testCase4Arr1 = {{1}, {2}};
        int[][] testCase5Arr1 = {{-5, -2, 0, 3}, {-3, -1, 2, 4}};
        
        System.out.println("Test Case 1:");
        System.out.println("Input: " + Arrays.toString(testCase1Arr1[0]) + ", " + Arrays.toString(testCase1Arr1[1]));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(testCase1Arr1[0], testCase1Arr1[1])));
        
        System.out.println("\\nTest Case 2:");
        System.out.println("Input: " + Arrays.toString(testCase2Arr1[0]) + ", " + Arrays.toString(testCase2Arr1[1]));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(testCase2Arr1[0], testCase2Arr1[1])));
        
        System.out.println("\\nTest Case 3:");
        System.out.println("Input: " + Arrays.toString(testCase3Arr1[0]) + ", " + Arrays.toString(testCase3Arr1[1]));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(testCase3Arr1[0], testCase3Arr1[1])));
        
        System.out.println("\\nTest Case 4:");
        System.out.println("Input: " + Arrays.toString(testCase4Arr1[0]) + ", " + Arrays.toString(testCase4Arr1[1]));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(testCase4Arr1[0], testCase4Arr1[1])));
        
        System.out.println("\\nTest Case 5:");
        System.out.println("Input: " + Arrays.toString(testCase5Arr1[0]) + ", " + Arrays.toString(testCase5Arr1[1]));
        System.out.println("Output: " + Arrays.toString(mergeSortedArrays(testCase5Arr1[0], testCase5Arr1[1])));
    }
}
