import java.util.*;

public class Task175 {

    interface MountainArray {
        int get(int k);
        int length();
    }

    static final class TestMountainArray implements MountainArray {
        private final int[] arr;
        private int getCount;
        private final int maxGets;

        TestMountainArray(int[] arr) {
            this(arr, Integer.MAX_VALUE);
        }

        TestMountainArray(int[] arr, int maxGets) {
            this.arr = arr != null ? Arrays.copyOf(arr, arr.length) : new int[0];
            this.getCount = 0;
            this.maxGets = Math.max(1, maxGets);
        }

        @Override
        public int get(int k) {
            if (k < 0 || k >= arr.length) {
                throw new IndexOutOfBoundsException("Index out of bounds");
            }
            if (getCount + 1 > maxGets) {
                throw new IllegalStateException("Exceeded get limit");
            }
            getCount++;
            return arr[k];
        }

        @Override
        public int length() {
            return arr.length;
        }

        public int getCount() {
            return getCount;
        }
    }

    public static int findInMountainArray(int target, MountainArray mountainArr) {
        if (mountainArr == null) return -1;
        int n = mountainArr.length();
        if (n < 3) return -1;

        int peak = findPeak(mountainArr);
        int left = binSearchAsc(mountainArr, 0, peak, target);
        if (left != -1) return left;
        return binSearchDesc(mountainArr, peak + 1, n - 1, target);
    }

    private static int findPeak(MountainArray m) {
        int lo = 0, hi = m.length() - 1;
        while (lo < hi) {
            int mid = lo + ((hi - lo) >>> 1);
            int a = m.get(mid);
            int b = m.get(mid + 1);
            if (a < b) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    private static int binSearchAsc(MountainArray m, int lo, int hi, int target) {
        int res = -1;
        while (lo <= hi) {
            int mid = lo + ((hi - lo) >>> 1);
            int val = m.get(mid);
            if (val == target) {
                res = mid;
                hi = mid - 1; // continue left for minimal index
            } else if (val < target) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return res;
    }

    private static int binSearchDesc(MountainArray m, int lo, int hi, int target) {
        int res = -1;
        while (lo <= hi) {
            int mid = lo + ((hi - lo) >>> 1);
            int val = m.get(mid);
            if (val == target) {
                res = mid;
                hi = mid - 1; // continue left for minimal index
            } else if (val > target) { // descending order
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return res;
    }

    public static void main(String[] args) {
        int[][] arrays = new int[][]{
            {1, 2, 3, 4, 5, 3, 1},
            {0, 1, 2, 4, 2, 1},
            {0, 5, 3, 1},
            {1, 5, 2},
            {1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1}
        };
        int[] targets = new int[]{3, 3, 1, 2, 7};

        for (int i = 0; i < arrays.length; i++) {
            TestMountainArray m = new TestMountainArray(arrays[i], 1000);
            int ans = findInMountainArray(targets[i], m);
            System.out.println(ans);
        }
    }
}