
import java.util.ArrayList;
import java.util.List;

public class Task21 {
    public static List<Integer> removeSmallest(List<Integer> numbers) {
        if (numbers == null || numbers.isEmpty()) {
            return new ArrayList<>();
        }
        
        List<Integer> result = new ArrayList<>(numbers);
        
        // Find the index of the smallest value
        int minIndex = 0;
        int minValue = result.get(0);
        
        for (int i = 1; i < result.size(); i++) {
            if (result.get(i) < minValue) {
                minValue = result.get(i);
                minIndex = i;
            }
        }
        
        // Remove the element at minIndex
        result.remove(minIndex);
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        List<Integer> test1 = new ArrayList<>(List.of(1, 2, 3, 4, 5));
        System.out.println("Input: " + test1 + ", Output: " + removeSmallest(test1));
        
        // Test case 2
        List<Integer> test2 = new ArrayList<>(List.of(5, 3, 2, 1, 4));
        System.out.println("Input: " + test2 + ", Output: " + removeSmallest(test2));
        
        // Test case 3
        List<Integer> test3 = new ArrayList<>(List.of(2, 2, 1, 2, 1));
        System.out.println("Input: " + test3 + ", Output: " + removeSmallest(test3));
        
        // Test case 4
        List<Integer> test4 = new ArrayList<>();
        System.out.println("Input: " + test4 + ", Output: " + removeSmallest(test4));
        
        // Test case 5
        List<Integer> test5 = new ArrayList<>(List.of(10));
        System.out.println("Input: " + test5 + ", Output: " + removeSmallest(test5));
    }
}
