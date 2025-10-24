import java.util.Locale;

public class Task185 {
    public static double findMedianSortedArrays(int[] nums1, int[] nums2) {
        int[] A = nums1 != null ? nums1 : new int[0];
        int[] B = nums2 != null ? nums2 : new int[0];

        // Ensure A is the smaller array
        if (A.length > B.length) {
            int[] tmp = A; A = B; B = tmp;
        }

        int m = A.length, n = B.length;
        int totalLeft = (m + n + 1) / 2;

        int lo = 0, hi = m;
        while (lo <= hi) {
            int i = lo + (hi - lo) / 2;
            int j = totalLeft - i;

            int leftA = (i == 0) ? Integer.MIN_VALUE : A[i - 1];
            int rightA = (i == m) ? Integer.MAX_VALUE : A[i];
            int leftB = (j == 0) ? Integer.MIN_VALUE : B[j - 1];
            int rightB = (j == n) ? Integer.MAX_VALUE : B[j];

            if (leftA <= rightB && leftB <= rightA) {
                if (((m + n) & 1) == 1) {
                    return Math.max(leftA, leftB);
                } else {
                    double a = Math.max(leftA, leftB);
                    double b = Math.min(rightA, rightB);
                    return (a + b) / 2.0;
                }
            } else if (leftA > rightB) {
                hi = i - 1;
            } else {
                lo = i + 1;
            }
        }
        throw new IllegalArgumentException("Invalid input arrays.");
    }

    public static void main(String[] args) {
        Locale.setDefault(Locale.US);
        System.out.println(String.format(Locale.US, "%.5f", findMedianSortedArrays(new int[]{1,3}, new int[]{2})));
        System.out.println(String.format(Locale.US, "%.5f", findMedianSortedArrays(new int[]{1,2}, new int[]{3,4})));
        System.out.println(String.format(Locale.US, "%.5f", findMedianSortedArrays(new int[]{}, new int[]{1})));
        System.out.println(String.format(Locale.US, "%.5f", findMedianSortedArrays(new int[]{0,0}, new int[]{0,0})));
        System.out.println(String.format(Locale.US, "%.5f", findMedianSortedArrays(new int[]{2}, new int[]{})));
    }
}