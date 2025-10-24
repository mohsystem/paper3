
import java.util.HashMap;
import java.util.Map;

public class Task26 {
    public static int findOddOccurrence(int[] arr) {
        if (arr == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        if (arr.length == 0) {
            throw new IllegalArgumentException("Array cannot be empty");
        }
        if (arr.length > 1000000) {
            throw new IllegalArgumentException("Array size exceeds maximum allowed");
        }
        
        Map<Integer, Integer> counts = new HashMap<>();
        
        for (int num : arr) {
            counts.put(num, counts.getOrDefault(num, 0) + 1);
        }
        
        for (Map.Entry<Integer, Integer> entry : counts.entrySet()) {
            if (entry.getValue() % 2 != 0) {
                return entry.getKey();
            }
        }
        
        throw new IllegalStateException("No number with odd occurrence found");
    }
    
    public static void main(String[] args) {
        int[][] testCases = {
            {7},
            {0},
            {1, 1, 2},
            {0, 1, 0, 1, 0},
            {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}
        };
        
        for (int i = 0; i < testCases.length; i++) {
            int result = findOddOccurrence(testCases[i]);
            System.out.println("Test case " + (i + 1) + ": " + result);
        }
    }
}
