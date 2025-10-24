// Chain-of-Through process:
// 1) Problem understanding: Compute inclusive sum between two integers a and b (unordered). If equal, return a.
// 2) Security requirements: Avoid overflow where feasible by using long and dividing an even factor before multiplication. No external input parsing.
// 3) Secure coding generation: Use arithmetic series formula with safe ordering. No loops over ranges to avoid performance issues.
// 4) Code review: Ensure integer division happens before large multiplication where one factor is guaranteed even.
// 5) Secure code output: Final function uses long, handles negative/positive values, returns result; includes 5 test cases.

public class Task10 {
    public static long getSum(int a, int b) {
        if (a == b) return a;
        long m = Math.min(a, b);
        long n = Math.max(a, b);
        long count = n - m + 1L;
        long s = m + n;
        if ((count & 1L) == 0L) {
            return (count / 2L) * s;
        } else {
            return count * (s / 2L);
        }
    }

    public static void main(String[] args) {
        int[][] tests = {
            {1, 0},
            {1, 2},
            {0, 1},
            {1, 1},
            {-1, 2}
        };
        for (int[] t : tests) {
            System.out.println(getSum(t[0], t[1]));
        }
    }
}