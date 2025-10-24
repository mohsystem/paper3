import java.util.Arrays;

public class Task145 {

    public static final class Result {
        public final boolean ok;
        public final long value;
        public final String error;

        private Result(boolean ok, long value, String error) {
            this.ok = ok;
            this.value = value;
            this.error = error;
        }

        public static Result success(long value) {
            return new Result(true, value, null);
        }

        public static Result failure(String error) {
            return new Result(false, 0L, error == null ? "error" : error);
        }
    }

    private static boolean willAddOverflow(long a, long b) {
        if (b > 0 && a > Long.MAX_VALUE - b) return true;
        if (b < 0 && a < Long.MIN_VALUE - b) return true;
        return false;
    }

    public static Result maxSubarraySum(int[] arr) {
        if (arr == null || arr.length == 0) {
            return Result.failure("invalid input: null or empty array");
        }

        long current = arr[0];
        long best = arr[0];

        for (int i = 1; i < arr.length; i++) {
            long v = arr[i];
            if (willAddOverflow(current, v)) {
                return Result.failure("sum overflow detected");
            }
            long sum = current + v;
            current = Math.max(v, sum);
            best = Math.max(best, current);
        }
        return Result.success(best);
    }

    private static void runTest(int testId, int[] input, Long expectedOrNull) {
        Result r = maxSubarraySum(input);
        System.out.println("Test " + testId + ": input=" + Arrays.toString(input));
        if (r.ok) {
            System.out.println("  ok=true, max_sum=" + r.value + (expectedOrNull != null ? ", expected=" + expectedOrNull : ""));
        } else {
            System.out.println("  ok=false, error=" + r.error);
        }
    }

    public static void main(String[] args) {
        int[] t1 = new int[]{-2, 1, -3, 4, -1, 2, 1, -5, 4}; // 6
        int[] t2 = new int[]{1, 2, 3, 4}; // 10
        int[] t3 = new int[]{-1, -2, -3}; // -1
        int[] t4 = new int[]{5, -9, 6, -2, 3}; // 7
        int[] t5 = new int[]{}; // error: empty

        runTest(1, t1, 6L);
        runTest(2, t2, 10L);
        runTest(3, t3, -1L);
        runTest(4, t4, 7L);
        runTest(5, t5, null);
    }
}