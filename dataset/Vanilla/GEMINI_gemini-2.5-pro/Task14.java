import java.util.Arrays;

public class Task14 {
    public static int findEvenIndex(int[] arr) {
        long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }

        long leftSum = 0;
        for (int i = 0; i < arr.length; i++) {
            // The right sum is the total sum minus the left sum and the current element
            long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            leftSum += arr[i];
        }

        return -1;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {1, 2, 3, 4, 3, 2, 1};
        System.out.println("Array: " + Arrays.toString(arr1) + " -> Index: " + findEvenIndex(arr1));

        // Test Case 2
        int[] arr2 = {1, 100, 50, -51, 1, 1};
        System.out.println("Array: " + Arrays.toString(arr2) + " -> Index: " + findEvenIndex(arr2));

        // Test Case 3
        int[] arr3 = {20, 10, -80, 10, 10, 15, 35};
        System.out.println("Array: " + Arrays.toString(arr3) + " -> Index: " + findEvenIndex(arr3));
        
        // Test Case 4
        int[] arr4 = {10, -80, 10, 10, 15, 35, 20};
        System.out.println("Array: " + Arrays.toString(arr4) + " -> Index: " + findEvenIndex(arr4));
        
        // Test Case 5
        int[] arr5 = {1, 2, 3, 4, 5, 6};
        System.out.println("Array: " + Arrays.toString(arr5) + " -> Index: " + findEvenIndex(arr5));
    }
}