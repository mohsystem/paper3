
import java.util.Arrays;

public class Task21 {
    public static int[] removeSmallest(int[] numbers) {
        // Handle null input
        if (numbers == null || numbers.length == 0) {
            return new int[0];
        }
        
        // Handle single element array
        if (numbers.length == 1) {
            return new int[0];
        }
        
        // Find the minimum value and its first index
        int minValue = numbers[0];
        int minIndex = 0;
        
        for (int i = 1; i < numbers.length; i++) {
            if (numbers[i] < minValue) {
                minValue = numbers[i];
                minIndex = i;
            }
        }
        
        // Create new array without mutating original
        int[] result = new int[numbers.length - 1];
        int resultIndex = 0;
        
        for (int i = 0; i < numbers.length; i++) {
            if (i != minIndex) {
                result[resultIndex++] = numbers[i];
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 3, 4, 5};
        System.out.println("Test 1: " + Arrays.toString(removeSmallest(test1)));
        
        // Test case 2
        int[] test2 = {5, 3, 2, 1, 4};
        System.out.println("Test 2: " + Arrays.toString(removeSmallest(test2)));
        
        // Test case 3
        int[] test3 = {2, 2, 1, 2, 1};
        System.out.println("Test 3: " + Arrays.toString(removeSmallest(test3)));
        
        // Test case 4
        int[] test4 = {};
        System.out.println("Test 4: " + Arrays.toString(removeSmallest(test4)));
        
        // Test case 5
        int[] test5 = {10};
        System.out.println("Test 5: " + Arrays.toString(removeSmallest(test5)));
    }
}
