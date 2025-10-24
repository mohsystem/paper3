
class MountainArray {
    private int[] arr;
    
    public MountainArray(int[] arr) {
        this.arr = arr;
    }
    
    public int get(int index) {
        if (index < 0 || index >= arr.length) {
            throw new IndexOutOfBoundsException("Index out of bounds");
        }
        return arr[index];
    }
    
    public int length() {
        return arr.length;
    }
}

class Task175 {
    public static int findInMountainArray(int target, MountainArray mountainArr) {
        if (mountainArr == null) {
            return -1;
        }
        
        int length = mountainArr.length();
        if (length < 3) {
            return -1;
        }
        
        int peakIndex = findPeak(mountainArr, length);
        
        int result = binarySearchAscending(mountainArr, target, 0, peakIndex);
        if (result != -1) {
            return result;
        }
        
        return binarySearchDescending(mountainArr, target, peakIndex + 1, length - 1);
    }
    
    private static int findPeak(MountainArray mountainArr, int length) {
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
    
    private static int binarySearchAscending(MountainArray mountainArr, int target, int left, int right) {
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
    
    private static int binarySearchDescending(MountainArray mountainArr, int target, int left, int right) {
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
    
    public static void main(String[] args) {
        int[][] testArrays = {
            {1, 2, 3, 4, 5, 3, 1},
            {0, 1, 2, 4, 2, 1},
            {1, 5, 2},
            {0, 5, 3, 1},
            {1, 2, 3, 5, 3}
        };
        int[] targets = {3, 3, 2, 1, 3};
        int[] expected = {2, -1, 2, 3, 2};
        
        for (int i = 0; i < testArrays.length; i++) {
            MountainArray mountainArr = new MountainArray(testArrays[i]);
            int result = findInMountainArray(targets[i], mountainArr);
            System.out.println("Test " + (i + 1) + ": target=" + targets[i] + 
                             ", result=" + result + ", expected=" + expected[i] + 
                             ", " + (result == expected[i] ? "PASS" : "FAIL"));
        }
    }
}
