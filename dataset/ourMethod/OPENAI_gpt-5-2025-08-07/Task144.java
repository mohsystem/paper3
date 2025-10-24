import java.util.Arrays;

public class Task144 {

    public static int[] mergeSorted(int[] a, int[] b) {
        if (a == null || b == null) {
            throw new IllegalArgumentException("Input arrays must not be null.");
        }
        if (!isNonDecreasing(a) || !isNonDecreasing(b)) {
            throw new IllegalArgumentException("Input arrays must be sorted in non-decreasing order.");
        }
        long totalLen = (long) a.length + (long) b.length;
        if (totalLen > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Combined array size is too large.");
        }

        int[] result = new int[(int) totalLen];
        int i = 0, j = 0, k = 0;
        while (i < a.length && j < b.length) {
            if (a[i] <= b[j]) {
                result[k++] = a[i++];
            } else {
                result[k++] = b[j++];
            }
        }
        while (i < a.length) {
            result[k++] = a[i++];
        }
        while (j < b.length) {
            result[k++] = b[j++];
        }
        return result;
    }

    private static boolean isNonDecreasing(int[] arr) {
        for (int i = 1; i < arr.length; i++) {
            if (arr[i - 1] > arr[i]) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        // Test case 1: both empty
        int[] t1a = new int[] {};
        int[] t1b = new int[] {};
        System.out.println(Arrays.toString(mergeSorted(t1a, t1b)));

        // Test case 2: one empty (b empty)
        int[] t2a = new int[] {1, 3, 5};
        int[] t2b = new int[] {};
        System.out.println(Arrays.toString(mergeSorted(t2a, t2b)));

        // Test case 3: one empty (a empty)
        int[] t3a = new int[] {};
        int[] t3b = new int[] {2, 4, 6};
        System.out.println(Arrays.toString(mergeSorted(t3a, t3b)));

        // Test case 4: duplicates
        int[] t4a = new int[] {1, 2, 2, 3};
        int[] t4b = new int[] {2, 2, 4};
        System.out.println(Arrays.toString(mergeSorted(t4a, t4b)));

        // Test case 5: negatives and positives
        int[] t5a = new int[] {-5, -2, 0};
        int[] t5b = new int[] {-3, -1, 2};
        System.out.println(Arrays.toString(mergeSorted(t5a, t5b)));
    }
}