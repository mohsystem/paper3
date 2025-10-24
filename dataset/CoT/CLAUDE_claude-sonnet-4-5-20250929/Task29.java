
import java.util.HashMap;
import java.util.Map;

public class Task29 {
    public static int[] twoSum(int[] numbers, int target) {
        if (numbers == null || numbers.length < 2) {
            return new int[0];
        }
        
        Map<Integer, Integer> map = new HashMap<>();
        
        for (int i = 0; i < numbers.length; i++) {
            int complement = target - numbers[i];
            
            if (map.containsKey(complement)) {
                return new int[] {map.get(complement), i};
            }
            
            map.put(numbers[i], i);
        }
        
        return new int[0];
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] result1 = twoSum(new int[] {1, 2, 3}, 4);
        System.out.println("Test 1: [" + result1[0] + ", " + result1[1] + "]");
        
        // Test case 2
        int[] result2 = twoSum(new int[] {3, 2, 4}, 6);
        System.out.println("Test 2: [" + result2[0] + ", " + result2[1] + "]");
        
        // Test case 3
        int[] result3 = twoSum(new int[] {1, 5, 3, 7, 9}, 12);
        System.out.println("Test 3: [" + result3[0] + ", " + result3[1] + "]");
        
        // Test case 4
        int[] result4 = twoSum(new int[] {10, 20, 30, 40}, 50);
        System.out.println("Test 4: [" + result4[0] + ", " + result4[1] + "]");
        
        // Test case 5
        int[] result5 = twoSum(new int[] {-1, -2, -3, -4}, -6);
        System.out.println("Test 5: [" + result5[0] + ", " + result5[1] + "]");
    }
}
