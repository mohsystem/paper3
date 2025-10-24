/**
 * // This is the MountainArray's API interface.
 * // You should not implement it, or speculate about its implementation
 * interface MountainArray {
 *     public int get(int index);
 *     public int length();
 * }
 */

// This is a dummy interface for compilation and testing purposes.
// In a real environment (like LeetCode), this would be provided.
interface MountainArray {
    public int get(int index);
    public int length();
}

// Dummy implementation for testing.
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
        
        // 1. Find the index of the peak element.
        int low = 0;
        int high = length - 1;
        while (low < high) {
            int mid = low + (high - low) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        int peakIndex = low;

        // 2. Search in the left (strictly increasing) part of the array.
        low = 0;
        high = peakIndex;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal == target) {
                return mid;
            } else if (midVal < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        // 3. If not found, search in the right (strictly decreasing) part.
        low = peakIndex + 1;
        high = length - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal == target) {
                return mid;
            } else if (midVal < target) { // In a decreasing array, smaller values are to the right.
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }
        
        // 4. Target not found in the array.
        return -1;
    }

    public static void main(String[] args) {
        Task175 solver = new Task175();

        // Test Case 1
        MountainArray mountainArr1 = new MountainArrayImpl(new int[]{1, 2, 3, 4, 5, 3, 1});
        int target1 = 3;
        System.out.println("Test 1 Output: " + solver.findInMountainArray(target1, mountainArr1)); // Expected: 2

        // Test Case 2
        MountainArray mountainArr2 = new MountainArrayImpl(new int[]{0, 1, 2, 4, 2, 1});
        int target2 = 3;
        System.out.println("Test 2 Output: " + solver.findInMountainArray(target2, mountainArr2)); // Expected: -1

        // Test Case 3
        MountainArray mountainArr3 = new MountainArrayImpl(new int[]{1, 5, 2});
        int target3 = 2;
        System.out.println("Test 3 Output: " + solver.findInMountainArray(target3, mountainArr3)); // Expected: 2
        
        // Test Case 4
        MountainArray mountainArr4 = new MountainArrayImpl(new int[]{3, 5, 3, 2, 0});
        int target4 = 0;
        System.out.println("Test 4 Output: " + solver.findInMountainArray(target4, mountainArr4)); // Expected: 4

        // Test Case 5
        MountainArray mountainArr5 = new MountainArrayImpl(new int[]{1, 5, 2});
        int target5 = 5;
        System.out.println("Test 5 Output: " + solver.findInMountainArray(target5, mountainArr5)); // Expected: 1
    }
}