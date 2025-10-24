import java.util.Arrays;

public class Task6 {
    public static double findUniq(double[] arr) {
        if (arr == null || arr.length < 3) throw new IllegalArgumentException("Array must have at least 3 elements");
        double a = arr[0], b = arr[1], c = arr[2];
        double common = (a == b) ? a : (a == c ? a : b);
        for (double v : arr) {
            if (v != common) return v;
        }
        return common; // Fallback (though problem guarantees one unique value)
    }

    public static void main(String[] args) {
        double[][] tests = new double[][]{
            new double[]{1, 1, 1, 2, 1, 1},
            new double[]{0, 0, 0.55, 0, 0},
            new double[]{-1, -1, -1, -2, -1},
            new double[]{3.14, 2.71, 3.14, 3.14, 3.14},
            new double[]{5, 5, 5, 5, 4}
        };

        for (double[] t : tests) {
            System.out.println(findUniq(t));
        }
    }
}