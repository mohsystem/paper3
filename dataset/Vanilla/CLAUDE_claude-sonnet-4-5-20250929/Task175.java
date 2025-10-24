
interface MountainArray {
    int get(int index);
    int length();
}

class MountainArrayImpl implements MountainArray {
    private int[] arr;
    
    public MountainArrayImpl(int[] arr) {
        this.arr = arr;
    }
    
    public int get(int index) {
        return arr[index];
    }
    
    public int length() {
        return arr.length;
    }
}

class Task175 {
    public static int findInMountainArray(int target, MountainArray mountainArr) {
        int length = mountainArr.length();
        
        // Find peak index
        int peakIndex = findPeak(mountainArr, length);
        
        // Search in ascending part
        int index = binarySearch(mountainArr, target, 0, peakIndex, true);
        if (index != -1) {
            return index;
        }
        
        // Search in descending part
        return binarySearch(mountainArr, target, peakIndex + 1, length - 1, false);
    }
    
    private static int findPeak(MountainArray mountainArr, int length) {
        int left = 0, right = length - 1;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        return left;
    }
    
    private static int binarySearch(MountainArray mountainArr, int target, int left, int right, boolean ascending) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int midVal = mountainArr.get(mid);
            
            if (midVal == target) {
                return mid;
            }
            
            if (ascending) {
                if (midVal < target) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            } else {
                if (midVal > target) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }
        }
        return -1;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] arr1 = {1, 2, 3, 4, 5, 3, 1};
        MountainArray ma1 = new MountainArrayImpl(arr1);
        System.out.println("Test 1: " + findInMountainArray(3, ma1)); // Expected: 2
        
        // Test case 2
        int[] arr2 = {0, 1, 2, 4, 2, 1};
        MountainArray ma2 = new MountainArrayImpl(arr2);
        System.out.println("Test 2: " + findInMountainArray(3, ma2)); // Expected: -1
        
        // Test case 3
        int[] arr3 = {1, 2, 3, 4, 5, 3, 1};
        MountainArray ma3 = new MountainArrayImpl(arr3);
        System.out.println("Test 3: " + findInMountainArray(5, ma3)); // Expected: 4
        
        // Test case 4
        int[] arr4 = {0, 5, 3, 1};
        MountainArray ma4 = new MountainArrayImpl(arr4);
        System.out.println("Test 4: " + findInMountainArray(1, ma4)); // Expected: 3
        
        // Test case 5
        int[] arr5 = {1, 5, 2};
        MountainArray ma5 = new MountainArrayImpl(arr5);
        System.out.println("Test 5: " + findInMountainArray(2, ma5)); // Expected: 2
    }
}
