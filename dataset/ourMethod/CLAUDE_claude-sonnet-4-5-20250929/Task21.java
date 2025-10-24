
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task21 {
    public static List<Integer> removeSmallest(List<Integer> numbers) {
        // Validate input
        if (numbers == null) {
            return new ArrayList<>();
        }
        
        // Return empty list if input is empty
        if (numbers.isEmpty()) {
            return new ArrayList<>();
        }
        
        // Find the minimum value
        int minValue = Integer.MAX_VALUE;
        for (Integer num : numbers) {
            if (num == null) {
                continue; // Skip null values
            }
            if (num < minValue) {
                minValue = num;
            }
        }
        
        // Create a new list without mutating the original
        List<Integer> result = new ArrayList<>();
        boolean removedFirst = false;
        
        for (Integer num : numbers) {
            if (num == null) {
                continue; // Skip null values
            }
            if (!removedFirst && num == minValue) {
                removedFirst = true;
                continue; // Skip the first occurrence of the minimum
            }
            result.add(num);
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        List<Integer> test1 = Arrays.asList(1, 2, 3, 4, 5);
        System.out.println("Test 1: " + removeSmallest(test1));
        
        // Test case 2
        List<Integer> test2 = Arrays.asList(5, 3, 2, 1, 4);
        System.out.println("Test 2: " + removeSmallest(test2));
        
        // Test case 3
        List<Integer> test3 = Arrays.asList(2, 2, 1, 2, 1);
        System.out.println("Test 3: " + removeSmallest(test3));
        
        // Test case 4: Empty array
        List<Integer> test4 = new ArrayList<>();
        System.out.println("Test 4: " + removeSmallest(test4));
        
        // Test case 5: Single element
        List<Integer> test5 = Arrays.asList(42);
        System.out.println("Test 5: " + removeSmallest(test5));
    }
}
