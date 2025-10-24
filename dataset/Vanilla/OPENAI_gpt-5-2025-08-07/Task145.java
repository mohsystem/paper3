import java.util.*;

public class Task145 {
    public static int maxSubArraySum(int[] nums) {
        if (nums == null || nums.length == 0) return 0;
        int current = nums[0], best = nums[0];
        for (int i = 1; i < nums.length; i++) {
            current = Math.max(nums[i], current + nums[i]);
            best = Math.max(best, current);
        }
        return best;
    }

    public static void main(String[] args) {
        int[][] tests = {
            {-2, 1, -3, 4, -1, 2, 1, -5, 4},
            {1},
            {5, 4, -1, 7, 8},
            {-1, -2, -3, -4},
            {0, 0, 0, 0}
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println(maxSubArraySum(tests[i]));
        }
    }
}