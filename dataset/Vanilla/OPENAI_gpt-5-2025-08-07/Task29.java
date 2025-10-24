import java.util.*;

public class Task29 {
    public static int[] two_sum(int[] numbers, int target) {
        Map<Integer, Integer> map = new HashMap<>();
        for (int i = 0; i < numbers.length; i++) {
            int complement = target - numbers[i];
            if (map.containsKey(complement)) {
                return new int[]{map.get(complement), i};
            }
            if (!map.containsKey(numbers[i])) {
                map.put(numbers[i], i);
            }
        }
        return new int[0];
    }

    private static void printResult(int[] res) {
        if (res.length == 2) {
            System.out.println("{" + res[0] + ", " + res[1] + "}");
        } else {
            System.out.println("{}");
        }
    }

    public static void main(String[] args) {
        int[][] tests = {
            {1, 2, 3},
            {3, 2, 4},
            {2, 7, 11, 15},
            {3, 3},
            {0, -1, 2, -3, 5}
        };
        int[] targets = {4, 6, 9, 6, -1};

        for (int i = 0; i < tests.length; i++) {
            int[] res = two_sum(tests[i], targets[i]);
            printResult(res);
        }
    }
}