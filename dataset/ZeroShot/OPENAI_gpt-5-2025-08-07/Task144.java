import java.util.Arrays;

public class Task144 {
    public static int[] mergeSorted(int[] a, int[] b) {
        if (a == null) a = new int[0];
        if (b == null) b = new int[0];

        long sum = (long) a.length + (long) b.length;
        if (sum > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("Merged array too large");
        }

        int[] result = new int[(int) sum];
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

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        int[] a1 = {};
        int[] b1 = {};

        int[] a2 = {1, 3, 5};
        int[] b2 = {2, 4, 6};

        int[] a3 = {};
        int[] b3 = {7, 8};

        int[] a4 = {-5, -1, 0, 2};
        int[] b4 = {-4, -3, 3, 10};

        int[] a5 = {1, 2, 2, 3};
        int[] b5 = {2, 2, 4, 5};

        printArray(mergeSorted(a1, b1));
        printArray(mergeSorted(a2, b2));
        printArray(mergeSorted(a3, b3));
        printArray(mergeSorted(a4, b4));
        printArray(mergeSorted(a5, b5));
    }
}