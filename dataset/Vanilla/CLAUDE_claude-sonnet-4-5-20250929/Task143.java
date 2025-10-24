
import java.util.Arrays;

public class Task143 {
    public static int[] sortArray(int[] arr) {
        if (arr == null || arr.length == 0) {
            return arr;
        }
        int[] result = arr.clone();
        Arrays.sort(result);
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Regular unsorted array
        int[] test1 = {5, 2, 8, 1, 9};
        System.out.println("Test 1: " + Arrays.toString(sortArray(test1)));
        
        // Test case 2: Already sorted array
        int[] test2 = {1, 2, 3, 4, 5};
        System.out.println("Test 2: " + Arrays.toString(sortArray(test2)));
        
        // Test case 3: Reverse sorted array
        int[] test3 = {9, 7, 5, 3, 1};
        System.out.println("Test 3: " + Arrays.toString(sortArray(test3)));
        
        // Test case 4: Array with duplicates
        int[] test4 = {4, 2, 7, 2, 9, 4};
        System.out.println("Test 4: " + Arrays.toString(sortArray(test4)));
        
        // Test case 5: Single element array
        int[] test5 = {42};
        System.out.println("Test 5: " + Arrays.toString(sortArray(test5)));
    }
}
