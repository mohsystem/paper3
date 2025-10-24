
import java.util.*;

public class Task166 {
    public static int[] posNegSort(int[] arr) {
        if (arr == null || arr.length == 0) {
            return new int[0];
        }
        
        // Extract positive numbers and sort them
        List<Integer> positives = new ArrayList<>();
        for (int num : arr) {
            if (num > 0) {
                positives.add(num);
            }
        }
        Collections.sort(positives);
        
        // Create result array and place sorted positives back
        int[] result = new int[arr.length];
        int posIndex = 0;
        
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] < 0) {
                result[i] = arr[i];
            } else {
                result[i] = positives.get(posIndex);
                posIndex++;
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {6, 3, -2, 5, -8, 2, -2};
        System.out.println("Test 1: " + Arrays.toString(posNegSort(test1)));
        
        // Test case 2
        int[] test2 = {6, 5, 4, -1, 3, 2, -1, 1};
        System.out.println("Test 2: " + Arrays.toString(posNegSort(test2)));
        
        // Test case 3
        int[] test3 = {-5, -5, -5, -5, 7, -5};
        System.out.println("Test 3: " + Arrays.toString(posNegSort(test3)));
        
        // Test case 4
        int[] test4 = {};
        System.out.println("Test 4: " + Arrays.toString(posNegSort(test4)));
        
        // Test case 5
        int[] test5 = {10, -3, 5, -1, 8, -7, 3};
        System.out.println("Test 5: " + Arrays.toString(posNegSort(test5)));
    }
}
