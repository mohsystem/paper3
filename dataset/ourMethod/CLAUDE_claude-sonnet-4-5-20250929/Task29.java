
import java.util.HashMap;
import java.util.Map;

public class Task29 {
    public static int[] twoSum(int[] numbers, int target) {
        if (numbers == null || numbers.length < 2) {
            throw new IllegalArgumentException("Array must contain at least 2 elements");
        }
        
        Map<Integer, Integer> map = new HashMap<>();
        
        for (int i = 0; i < numbers.length; i++) {
            int complement = target - numbers[i];
            
            if (map.containsKey(complement)) {
                return new int[]{map.get(complement), i};
            }
            
            map.put(numbers[i], i);
        }
        
        throw new IllegalArgumentException("No valid pair found");
    }
    
    public static void main(String[] args) {
        int[] result1 = twoSum(new int[]{1, 2, 3}, 4);
        System.out.println("[" + result1[0] + ", " + result1[1] + "]");
        
        int[] result2 = twoSum(new int[]{3, 2, 4}, 6);
        System.out.println("[" + result2[0] + ", " + result2[1] + "]");
        
        int[] result3 = twoSum(new int[]{1, 5, 3, 7, 9}, 12);
        System.out.println("[" + result3[0] + ", " + result3[1] + "]");
        
        int[] result4 = twoSum(new int[]{10, 20, 30, 40}, 50);
        System.out.println("[" + result4[0] + ", " + result4[1] + "]");
        
        int[] result5 = twoSum(new int[]{-1, -2, -3, -4, -5}, -8);
        System.out.println("[" + result5[0] + ", " + result5[1] + "]");
    }
}
