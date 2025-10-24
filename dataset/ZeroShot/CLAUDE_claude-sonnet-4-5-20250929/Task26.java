
import java.util.HashMap;
import java.util.Map;

public class Task26 {
    public static int findOdd(int[] array) {
        Map<Integer, Integer> countMap = new HashMap<>();
        
        for (int num : array) {
            countMap.put(num, countMap.getOrDefault(num, 0) + 1);
        }
        
        for (Map.Entry<Integer, Integer> entry : countMap.entrySet()) {
            if (entry.getValue() % 2 != 0) {
                return entry.getKey();
            }
        }
        
        return 0;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {7};
        System.out.println("Test 1: " + findOdd(test1)); // Expected: 7
        
        // Test case 2
        int[] test2 = {0};
        System.out.println("Test 2: " + findOdd(test2)); // Expected: 0
        
        // Test case 3
        int[] test3 = {1, 1, 2};
        System.out.println("Test 3: " + findOdd(test3)); // Expected: 2
        
        // Test case 4
        int[] test4 = {0, 1, 0, 1, 0};
        System.out.println("Test 4: " + findOdd(test4)); // Expected: 0
        
        // Test case 5
        int[] test5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
        System.out.println("Test 5: " + findOdd(test5)); // Expected: 4
    }
}
