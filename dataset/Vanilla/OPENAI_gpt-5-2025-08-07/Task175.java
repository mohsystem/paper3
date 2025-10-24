import java.util.*;

interface MountainArray {
    public int get(int index);
    public int length();
}

class SimpleMountainArray implements MountainArray {
    private int[] arr;
    private int calls;

    public SimpleMountainArray(int[] arr) {
        this.arr = arr;
        this.calls = 0;
    }

    public int get(int index) {
        calls++;
        return arr[index];
    }

    public int length() {
        return arr.length;
    }

    public int getCalls() {
        return calls;
    }
}

public class Task175 {
    public static int findInMountainArray(int target, MountainArray mountainArr) {
        int n = mountainArr.length();
        int peak = findPeak(mountainArr, n);
        int left = binarySearchAsc(mountainArr, 0, peak, target);
        if (left != -1) return left;
        return binarySearchDesc(mountainArr, peak + 1, n - 1, target);
    }

    private static int findPeak(MountainArray m, int n) {
        int l = 0, r = n - 1;
        while (l < r) {
            int mid = l + (r - l) / 2;
            int a = m.get(mid);
            int b = m.get(mid + 1);
            if (a < b) {
                l = mid + 1;
            } else {
                r = mid;
            }
        }
        return l;
    }

    private static int binarySearchAsc(MountainArray m, int l, int r, int target) {
        while (l <= r) {
            int mid = l + (r - l) / 2;
            int val = m.get(mid);
            if (val == target) return mid;
            if (val < target) l = mid + 1;
            else r = mid - 1;
        }
        return -1;
    }

    private static int binarySearchDesc(MountainArray m, int l, int r, int target) {
        while (l <= r) {
            int mid = l + (r - l) / 2;
            int val = m.get(mid);
            if (val == target) return mid;
            if (val < target) r = mid - 1;
            else l = mid + 1;
        }
        return -1;
    }

    public static void main(String[] args) {
        int[][] arrays = {
            {1,2,3,4,5,3,1},
            {0,1,2,4,2,1},
            {0,5,3,1},
            {0,2,4,5,3,1},
            {0,2,4,5,3,1}
        };
        int[] targets = {3, 3, 1, 0, 1};
        for (int i = 0; i < arrays.length; i++) {
            SimpleMountainArray sma = new SimpleMountainArray(arrays[i]);
            int res = findInMountainArray(targets[i], sma);
            System.out.println(res);
        }
    }
}