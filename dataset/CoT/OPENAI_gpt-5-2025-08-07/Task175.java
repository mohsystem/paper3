import java.util.Arrays;

public class Task175 {
    // MountainArray interface simulation with get-call counting
    static final class MountainArray {
        private final int[] data;
        private int calls;

        MountainArray(int[] source) {
            if (source == null) throw new IllegalArgumentException("Source array cannot be null");
            this.data = Arrays.copyOf(source, source.length);
            this.calls = 0;
        }

        public int get(int k) {
            if (k < 0 || k >= data.length) throw new IndexOutOfBoundsException("Index out of bounds: " + k);
            calls++;
            return data[k];
        }

        public int length() {
            return data.length;
        }

        public int getCalls() {
            return calls;
        }
    }

    public static int findInMountainArrayWithRawArray(int[] array, int target) {
        MountainArray m = new MountainArray(array);
        return findInMountainArray(target, m);
    }

    public static int findInMountainArray(int target, MountainArray mountainArr) {
        int n = mountainArr.length();
        if (n < 3) return -1;

        int peak = findPeakIndex(mountainArr);

        int leftAns = binarySearchAsc(mountainArr, 0, peak, target);
        if (leftAns != -1) return leftAns;

        return binarySearchDesc(mountainArr, peak + 1, n - 1, target);
    }

    private static int findPeakIndex(MountainArray m) {
        int lo = 0, hi = m.length() - 1;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
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

    private static int binarySearchAsc(MountainArray m, int lo, int hi, int target) {
        int ans = -1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            int val = m.get(mid);
            if (val == target) {
                ans = mid;
                hi = mid - 1; // find leftmost
            } else if (val < target) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return ans;
    }

    private static int binarySearchDesc(MountainArray m, int lo, int hi, int target) {
        int ans = -1;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            int val = m.get(mid);
            if (val == target) {
                ans = mid;
                hi = mid - 1; // find leftmost within descending segment
            } else if (val < target) { // move left where values are larger
                hi = mid - 1;
            } else {
                lo = mid + 1;
            }
        }
        return ans;
    }

    // 5 test cases
    public static void main(String[] args) {
        int[][] arrays = {
                {1,2,3,4,5,3,1},
                {0,1,2,4,2,1},
                {0,5,3,1},
                {1,2,3,4,5,4,3,2,1},
                {0,1,0}
        };
        int[] targets = {3, 3, 1, 9, 0};
        for (int i = 0; i < arrays.length; i++) {
            int res = findInMountainArrayWithRawArray(arrays[i], targets[i]);
            System.out.println(res);
        }
    }
}