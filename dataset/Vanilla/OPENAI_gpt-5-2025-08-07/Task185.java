import java.util.*;

public class Task185 {
    public static double findMedianSortedArrays(int[] nums1, int[] nums2) {
        if (nums1.length > nums2.length) return findMedianSortedArrays(nums2, nums1);
        int m = nums1.length, n = nums2.length;
        int imin = 0, imax = m, half = (m + n + 1) / 2;
        while (imin <= imax) {
            int i = (imin + imax) / 2;
            int j = half - i;
            if (i < m && j > 0 && nums2[j - 1] > nums1[i]) {
                imin = i + 1;
            } else if (i > 0 && j < n && nums1[i - 1] > nums2[j]) {
                imax = i - 1;
            } else {
                int maxLeft;
                if (i == 0) maxLeft = nums2[j - 1];
                else if (j == 0) maxLeft = nums1[i - 1];
                else maxLeft = Math.max(nums1[i - 1], nums2[j - 1]);
                if (((m + n) % 2) == 1) return maxLeft;
                int minRight;
                if (i == m) minRight = nums2[j];
                else if (j == n) minRight = nums1[i];
                else minRight = Math.min(nums1[i], nums2[j]);
                return (maxLeft + minRight) / 2.0;
            }
        }
        return 0.0;
    }

    public static void main(String[] args) {
        int[] a1 = {1, 3}; int[] b1 = {2};
        int[] a2 = {1, 2}; int[] b2 = {3, 4};
        int[] a3 = {};     int[] b3 = {1};
        int[] a4 = {0, 0}; int[] b4 = {0, 0};
        int[] a5 = {2};    int[] b5 = {};

        double r1 = findMedianSortedArrays(a1, b1);
        double r2 = findMedianSortedArrays(a2, b2);
        double r3 = findMedianSortedArrays(a3, b3);
        double r4 = findMedianSortedArrays(a4, b4);
        double r5 = findMedianSortedArrays(a5, b5);

        System.out.printf("%.5f%n", r1);
        System.out.printf("%.5f%n", r2);
        System.out.printf("%.5f%n", r3);
        System.out.printf("%.5f%n", r4);
        System.out.printf("%.5f%n", r5);
    }
}