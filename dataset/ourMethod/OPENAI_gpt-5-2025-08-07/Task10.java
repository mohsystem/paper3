public class Task10 {
    public static long getSum(int a, int b) {
        long start = Math.min((long) a, (long) b);
        long end = Math.max((long) a, (long) b);
        long n = end - start + 1L; // number of terms
        long sumPair = start + end; // first + last
        if ((n & 1L) == 0L) {
            return (n / 2L) * sumPair; // n is even
        } else {
            return n * (sumPair / 2L); // sumPair is even
        }
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            {1, 0},
            {1, 2},
            {0, 1},
            {1, 1},
            {-1, 2}
        };
        for (int[] t : tests) {
            long result = getSum(t[0], t[1]);
            System.out.println(result);
        }
    }
}