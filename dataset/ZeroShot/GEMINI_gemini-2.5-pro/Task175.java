/**
 * // This is the MountainArray's API interface.
 * // You should not implement it, or speculate about its implementation
 * interface MountainArray {
 *     public int get(int index) {}
 *     public int length() {}
 * }
 */
interface MountainArray {
    public int get(int index);
    public int length();
}

// Concrete implementation for testing purposes
class MountainArrayImpl implements MountainArray {
    private final int[] arr;

    public MountainArrayImpl(int[] arr) {
        this.arr = arr;
    }

    @Override
    public int get(int index) {
        return arr[index];
    }

    @Override
    public int length() {
        return arr.length;
    }
}

public class Task175 {

    public int findInMountainArray(int target, MountainArray mountainArr) {
        int length = mountainArr.length();

        // 1. Find the peak index
        int low = 0;
        int high = length - 1;
        int peakIndex = -1;
        while (low < high) {
            int mid = low + (high - low) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        peakIndex = low;

        // 2. Search in the ascending part [0, peakIndex]
        low = 0;
        high = peakIndex;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int val = mountainArr.get(mid);
            if (val == target) {
                return mid;
            } else if (val < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        // 3. Search in the descending part [peakIndex + 1, length - 1]
        low = peakIndex + 1;
        high = length - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int val = mountainArr.get(mid);
            if (val == target) {
                return mid;
            } else if (val < target) { // descending, so move to smaller indices (left)
                high = mid - 1;
            } else { // val > target, move to larger indices (right)
                low = mid + 1;
            }
        }

        return -1; // Target not found
    }

    public static void main(String[] args) {
        Task175 solution = new Task175();

        // Test Case 1
        MountainArray mountainArr1 = new MountainArrayImpl(new int[]{1, 2, 3, 4, 5, 3, 1});
        int target1 = 3;
        System.out.println("Test Case 1: " + solution.findInMountainArray(target1, mountainArr1));

        // Test Case 2
        MountainArray mountainArr2 = new MountainArrayImpl(new int[]{0, 1, 2, 4, 2, 1});
        int target2 = 3;
        System.out.println("Test Case 2: " + solution.findInMountainArray(target2, mountainArr2));

        // Test Case 3
        MountainArray mountainArr3 = new MountainArrayImpl(new int[]{1, 5, 2});
        int target3 = 2;
        System.out.println("Test Case 3: " + solution.findInMountainArray(target3, mountainArr3));
        
        // Test Case 4
        MountainArray mountainArr4 = new MountainArrayImpl(new int[]{1, 2, 5, 2, 1});
        int target4 = 1;
        System.out.println("Test Case 4: " + solution.findInMountainArray(target4, mountainArr4));
        
        // Test Case 5
        MountainArray mountainArr5 = new MountainArrayImpl(new int[]{1, 2, 5, 2, 1});
        int target5 = 5;
        System.out.println("Test Case 5: " + solution.findInMountainArray(target5, mountainArr5));
    }
}