import java.util.ArrayList;
import java.util.Collections;
import java.util.Arrays;

public class Task166 {

    public static int[] posNegSort(int[] arr) {
        if (arr == null || arr.length == 0) {
            return new int[0];
        }

        // 1. Extract positive numbers
        ArrayList<Integer> positives = new ArrayList<>();
        for (int num : arr) {
            if (num > 0) {
                positives.add(num);
            }
        }

        // 2. Sort the positive numbers
        Collections.sort(positives);

        // 3. Place sorted positives back into the array
        int posIndex = 0;
        // The original array 'arr' is modified in place
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] > 0) {
                arr[i] = positives.get(posIndex++);
            }
        }

        return arr;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {6, 3, -2, 5, -8, 2, -2};
        System.out.println("Test 1 Input: " + Arrays.toString(arr1));
        System.out.println("Test 1 Output: " + Arrays.toString(posNegSort(arr1)));

        // Test Case 2
        int[] arr2 = {6, 5, 4, -1, 3, 2, -1, 1};
        System.out.println("Test 2 Input: " + Arrays.toString(arr2));
        System.out.println("Test 2 Output: " + Arrays.toString(posNegSort(arr2)));

        // Test Case 3
        int[] arr3 = {-5, -5, -5, -5, 7, -5};
        System.out.println("Test 3 Input: " + Arrays.toString(arr3));
        System.out.println("Test 3 Output: " + Arrays.toString(posNegSort(arr3)));

        // Test Case 4
        int[] arr4 = {};
        System.out.println("Test 4 Input: " + Arrays.toString(arr4));
        System.out.println("Test 4 Output: " + Arrays.toString(posNegSort(arr4)));

        // Test Case 5
        int[] arr5 = {-1, -2, -3, 1, 2, 3};
        System.out.println("Test 5 Input: " + Arrays.toString(arr5));
        System.out.println("Test 5 Output: " + Arrays.toString(posNegSort(arr5)));
    }
}