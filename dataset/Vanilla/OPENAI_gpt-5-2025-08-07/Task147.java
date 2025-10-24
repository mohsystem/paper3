import java.util.Arrays;

public class Task147 {
    public static int kthLargest(int[] arr, int k) {
        if (arr == null || k < 1 || k > arr.length) {
            throw new IllegalArgumentException("Invalid k or array");
        }
        int[] copy = Arrays.copyOf(arr, arr.length);
        Arrays.sort(copy);
        return copy[copy.length - k];
    }

    public static void main(String[] args) {
        int[][] testArrays = {
            {3, 2, 1, 5, 6, 4},
            {3, 2, 3, 1, 2, 4, 5, 5, 6},
            {7},
            {-1, -2, -3, -4},
            {5, 5, 5, 5}
        };
        int[] ks = {2, 4, 1, 2, 3};

        for (int i = 0; i < 5; i++) {
            int result = kthLargest(testArrays[i], ks[i]);
            System.out.println(result);
        }
    }
}