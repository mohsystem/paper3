import java.util.Arrays;

interface MountainArray {
    int get(int index);
    int length();
    int getCallCount();
}

final class ArrayMountain implements MountainArray {
    private final int[] data;
    private int calls;

    ArrayMountain(int[] arr) {
        if (arr == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        this.data = Arrays.copyOf(arr, arr.length);
        this.calls = 0;
    }

    @Override
    public int get(int index) {
        if (index < 0 || index >= data.length) {
            throw new IllegalArgumentException("Index out of bounds");
        }
        calls++;
        return data[index];
    }

    @Override
    public int length() {
        return data.length;
    }

    @Override
    public int getCallCount() {
        return calls;
    }
}

public class Task175 {
    public static int findInMountainArray(int target, MountainArray mountainArr) {
        if (mountainArr == null) return -1;
        int n = mountainArr.length();
        if (n < 3) return -1;

        int peak = findPeak(mountainArr);
        int left = binarySearchAsc(mountainArr, 0, peak, target);
        if (left != -1) return left;
        return binarySearchDesc(mountainArr, peak + 1, n - 1, target);
    }

    private static int findPeak(MountainArray arr) {
        int l = 0, r = arr.length() - 1;
        while (l < r) {
            int m = l + (r - l) / 2;
            int a = arr.get(m);
            int b = arr.get(m + 1);
            if (a < b) {
                l = m + 1;
            } else {
                r = m;
            }
        }
        return l;
    }

    private static int binarySearchAsc(MountainArray arr, int l, int r, int target) {
        while (l <= r) {
            int m = l + (r - l) / 2;
            int v = arr.get(m);
            if (v == target) return m;
            if (v < target) {
                l = m + 1;
            } else {
                r = m - 1;
            }
        }
        return -1;
    }

    private static int binarySearchDesc(MountainArray arr, int l, int r, int target) {
        while (l <= r) {
            int m = l + (r - l) / 2;
            int v = arr.get(m);
            if (v == target) return m;
            if (v < target) {
                r = m - 1;
            } else {
                l = m + 1;
            }
        }
        return -1;
    }

    private static void runTest(int[] array, int target, int expected) {
        ArrayMountain ma = new ArrayMountain(array);
        int result = findInMountainArray(target, ma);
        System.out.println("Array: " + Arrays.toString(array) + ", target=" + target +
                " -> result=" + result + ", expected=" + expected + ", getCalls=" + ma.getCallCount());
    }

    public static void main(String[] args) {
        runTest(new int[]{1, 2, 3, 4, 5, 3, 1}, 3, 2);        // Example 1
        runTest(new int[]{0, 1, 2, 4, 2, 1}, 3, -1);           // Example 2
        runTest(new int[]{1, 3, 5, 7, 9, 6, 4, 2}, 1, 0);      // Target at start
        runTest(new int[]{2, 5, 9, 12, 10, 7, 2}, 7, 5);       // Target on right
        runTest(new int[]{0, 2, 5, 10, 9, 8, 1}, 10, 3);       // Target at peak
    }
}