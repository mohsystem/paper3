
// MountainArray interface (provided by the problem)
interface MountainArray {
    int get(int index);
    int length();
}

class Task175 {
    public int findInMountainArray(int target, MountainArray mountainArr) {
        // Input validation
        if (mountainArr == null) {
            return -1;
        }
        
        int length = mountainArr.length();
        if (length < 3) {
            return -1;
        }
        
        // Step 1: Find the peak index
        int peakIndex = findPeak(mountainArr, length);
        
        // Step 2: Search in the ascending part (left side)
        int result = binarySearchAscending(mountainArr, target, 0, peakIndex);
        
        // Step 3: If not found, search in the descending part (right side)
        if (result == -1) {
            result = binarySearchDescending(mountainArr, target, peakIndex + 1, length - 1);
        }
        
        return result;
    }
    
    private int findPeak(MountainArray mountainArr, int length) {
        int left = 0;
        int right = length - 1;
        
        while (left < right) {
            int mid = left + (right - left) / 2;
            int midVal = mountainArr.get(mid);
            int nextVal = mountainArr.get(mid + 1);
            
            if (midVal < nextVal) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        
        return left;
    }
    
    private int binarySearchAscending(MountainArray mountainArr, int target, int left, int right) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int midVal = mountainArr.get(mid);
            
            if (midVal == target) {
                return mid;
            } else if (midVal < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return -1;
    }
    
    private int binarySearchDescending(MountainArray mountainArr, int target, int left, int right) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int midVal = mountainArr.get(mid);
            
            if (midVal == target) {
                return mid;
            } else if (midVal > target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return -1;
    }
    
    // Test implementation of MountainArray
    static class TestMountainArray implements MountainArray {
        private int[] arr;
        
        public TestMountainArray(int[] arr) {
            this.arr = arr;
        }
        
        public int get(int index) {
            return arr[index];
        }
        
        public int length() {
            return arr.length;
        }
    }
    
    public static void main(String[] args) {
        Task175 solution = new Task175();
        
        // Test case 1
        MountainArray test1 = new TestMountainArray(new int[]{1, 2, 3, 4, 5, 3, 1});
        System.out.println("Test 1: " + solution.findInMountainArray(3, test1)); // Expected: 2
        
        // Test case 2
        MountainArray test2 = new TestMountainArray(new int[]{0, 1, 2, 4, 2, 1});
        System.out.println("Test 2: " + solution.findInMountainArray(3, test2)); // Expected: -1
        
        // Test case 3
        MountainArray test3 = new TestMountainArray(new int[]{1, 2, 3, 4, 5, 3, 1});
        System.out.println("Test 3: " + solution.findInMountainArray(5, test3)); // Expected: 4
        
        // Test case 4
        MountainArray test4 = new TestMountainArray(new int[]{0, 5, 3, 1});
        System.out.println("Test 4: " + solution.findInMountainArray(1, test4)); // Expected: 3
        
        // Test case 5
        MountainArray test5 = new TestMountainArray(new int[]{1, 5, 2});
        System.out.println("Test 5: " + solution.findInMountainArray(2, test5)); // Expected: 2
    }
}
