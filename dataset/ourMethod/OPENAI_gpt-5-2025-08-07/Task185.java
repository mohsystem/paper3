import java.util.Arrays;

public class Task185 {
    // Returns the median of two sorted arrays in O(log(m+n)) time.
    public static double findMedianSortedArrays(int[] nums1, int[] nums2) {
        int[] A = (nums1 == null) ? new int[0] : nums1;
        int[] B = (nums2 == null) ? new int[0] : nums2;

        if (A.length == 0 && B.length == 0) {
            throw new IllegalArgumentException("Both input arrays are empty.");
        }

        // Ensure A is the smaller array to minimize the binary search range
        if (A.length > B.length) {
            int[] tmp = A; A = B; B = tmp;
        }

        int m = A.length;
        int n = B.length;
        int low = 0, high = m;
        int half = (m + n + 1) / 2;

        while (low <= high) {
            int i = (low + high) / 2;
            int j = half - i;

            int Aleft  = (i == 0) ? Integer.MIN_VALUE : A[i - 1];
            int Aright = (i == m) ? Integer.MAX_VALUE : A[i];
            int Bleft  = (j == 0) ? Integer.MIN_VALUE : B[j - 1];
            int Bright = (j == n) ? Integer.MAX_VALUE : B[j];

            if (Aleft <= Bright && Bleft <= Aright) {
                if (((m + n) & 1) == 0) {
                    int leftMax = Math.max(Aleft, Bleft);
                    int rightMin = Math.min(Aright, Bright);
                    return (leftMax + rightMin) / 2.0;
                } else {
                    return (double)Math.max(Aleft, Bleft);
                }
            } else if (Aleft > Bright) {
                high = i - 1;
            } else {
                low = i + 1;
            }
        }

        throw new IllegalArgumentException("Input arrays must be sorted.");
    }

    private static void runTest(int[] a, int[] b) {
        double median = findMedianSortedArrays(a, b);
        System.out.printf("%.5f%n", median);
    }

    public static void main(String[] args) {
        // 5 Test cases
        runTest(new int[]{1, 3}, new int[]{2});                       // 2.00000
        runTest(new int[]{1, 2}, new int[]{3, 4});                    // 2.50000
        runTest(new int[]{}, new int[]{1});                           // 1.00000
        runTest(new int[]{0, 0}, new int[]{0, 0});                    // 0.00000
        runTest(new int[]{-1000000, -5, 0, 7}, new int[]{-3, 2, 2, 1000000}); // 1.00000
    }
}