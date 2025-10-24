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

class MountainArrayImpl implements MountainArray {
    private int[] arr;
    private int get_calls = 0;

    public MountainArrayImpl(int[] arr) {
        this.arr = arr;
    }

    @Override
    public int get(int index) {
        get_calls++;
        // You can uncomment the line below to check the number of calls
        // System.out.println("GET call #" + get_calls + " for index " + index);
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

        // 1. Find the index of the peak element
        int low = 0;
        int high = length - 1;
        int peak = -1;
        while (low < high) {
            int mid = low + (high - low) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        peak = low;

        // 2. Search in the left (ascending) subarray
        low = 0;
        high = peak;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal < target) {
                low = mid + 1;
            } else if (midVal > target) {
                high = mid - 1;
            } else {
                return mid; // Target found
            }
        }

        // 3. Search in the right (descending) subarray
        low = peak + 1;
        high = length - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal > target) {
                low = mid + 1;
            } else if (midVal < target) {
                high = mid - 1;
            } else {
                return mid; // Target found
            }
        }
        
        // 4. Target not found
        return -1;
    }
    
    public static void main(String[] args) {
        Task175 solution = new Task175();

        // Test Case 1
        MountainArray ma1 = new MountainArrayImpl(new int[]{1, 2, 3, 4, 5, 3, 1});
        int target1 = 3;
        System.out.println("Test Case 1: " + solution.findInMountainArray(target1, ma1)); // Expected: 2

        // Test Case 2
        MountainArray ma2 = new MountainArrayImpl(new int[]{0, 1, 2, 4, 2, 1});
        int target2 = 3;
        System.out.println("Test Case 2: " + solution.findInMountainArray(target2, ma2)); // Expected: -1

        // Test Case 3
        MountainArray ma3 = new MountainArrayImpl(new int[]{1, 5, 2});
        int target3 = 2;
        System.out.println("Test Case 3: " + solution.findInMountainArray(target3, ma3)); // Expected: 2
        
        // Test Case 4
        MountainArray ma4 = new MountainArrayImpl(new int[]{1, 2, 5, 4, 3});
        int target4 = 5;
        System.out.println("Test Case 4: " + solution.findInMountainArray(target4, ma4)); // Expected: 2

        // Test Case 5
        MountainArray ma5 = new MountainArrayImpl(new int[]{3, 5, 3, 2, 0});
        int target5 = 0;
        System.out.println("Test Case 5: " + solution.findInMountainArray(target5, ma5)); // Expected: 4
    }
}