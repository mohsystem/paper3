public class Task14 {
    public static int findEvenIndex(int[] arr) {
        if (arr == null) return -1;
        int n = arr.length;
        if (n < 1 || n >= 1000) return -1;

        long total = 0L;
        for (int v : arr) {
            total += (long) v;
        }

        long left = 0L;
        for (int i = 0; i < n; i++) {
            long right = total - left - (long) arr[i];
            if (left == right) {
                return i;
            }
            left += (long) arr[i];
        }
        return -1;
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{1, 2, 3, 4, 3, 2, 1},
            new int[]{1, 100, 50, -51, 1, 1},
            new int[]{20, 10, -80, 10, 10, 15, 35},
            new int[]{1, 2, 3, 4, 5, 6},
            new int[]{0, 0, 0, 0}
        };
        for (int i = 0; i < tests.length; i++) {
            int result = findEvenIndex(tests[i]);
            System.out.println(result);
        }
    }
}