import java.util.HashMap;
import java.util.Map;

public class Task29 {
    public static int[] twoSum(int[] numbers, int target) {
        if (numbers == null || numbers.length < 2) {
            throw new IllegalArgumentException("Invalid input");
        }
        Map<Integer, Integer> map = new HashMap<>();
        for (int i = 0; i < numbers.length; i++) {
            int num = numbers[i];
            int complement = target - num;
            Integer idx = map.get(complement);
            if (idx != null) {
                return new int[] { idx, i };
            }
            // keep earliest index for duplicates
            map.putIfAbsent(num, i);
        }
        throw new IllegalArgumentException("No two sum solution");
    }

    private static void printPair(int[] pair) {
        System.out.println("{" + pair[0] + ", " + pair[1] + "}");
    }

    public static void main(String[] args) {
        printPair(twoSum(new int[]{1, 2, 3}, 4));        // {0, 2}
        printPair(twoSum(new int[]{3, 2, 4}, 6));        // {1, 2}
        printPair(twoSum(new int[]{3, 3}, 6));           // {0, 1}
        printPair(twoSum(new int[]{-1, -2, -3, -4}, -6));// {1, 3} or valid
        printPair(twoSum(new int[]{2, 7, 11, 15}, 9));   // {0, 1}
    }
}